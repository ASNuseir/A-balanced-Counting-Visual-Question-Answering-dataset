/*MainShell.cpp
* Defines the Main Body of the Generator and especially the Main Loop
*/

#ifdef _WIN32
    #define _USE_MATH_DEFINES // for C++
    #include<windows.h>
#endif
#include<osg/Node>
#include<osg/ref_ptr>
#include<osg/Geode>
#include<osg/Group>
#include<osg/MatrixTransform>
#include<osg/Matrix>
#include<osg/Material>
#include<osgDB/ReadFile>
#include<osgDB/WriteFile>
#include<osgViewer/Viewer>
#include<osgGA/TrackballManipulator>
#include<osgShadow/ShadowedScene>
#include<osgShadow/ShadowMap>

#include<string>
#include<vector>
#include<random>
#include<iostream>
#include<fstream>
#include<exception>
#include<cmath>
#include"CLI11.hpp"

#include"Utils.hpp"
#include"UpdateStep.h"
#include"Init.h"

using osg::ref_ptr;
using osg::Node;
using osg::Matrix;
using osg::MatrixTransform;
using osg::Group;

using std::vector;
using std::string;
using std::cout;
using std::cin;

int main(int argc, char* argv[]) {
    Config config;
    std::mt19937 RNGStream;
    {
        CLI::App app{ "Tool to generate datasets to test counting in VisualQA." };
        app.add_option("-t,--target", config.target, "File containing the objects to load.");
        app.add_option("-f,--floor", config.tileColors, "File containing colors for floor tiles. Floor tiles will be randomly colored from this pool.");
        app.add_flag("-v,--validation", config.val, "If set to true, label set as validation instead.");
        app.add_option("-s,--seed", config.seed, "Seed from which the datset is generated");

        app.add_option("-c,--constellations", config.constellations, "Number of different board states for each object count");
        app.add_option("-b,--boardsize", config.bsize, "Edge length of the board in tiles");
        app.add_option("-o,--offset", config.stepsize, "Distance between tile centers");

        app.add_option("--aBias", config.aBias, "Ambient Bias for shadow casting");
        app.add_option("--aBright", config.aBright, "Ambient Light Brightness factor");
        app.add_flag("--extDebug", config.extDebug, "Sets extended Debug output");

        app.add_option("-i,--identifier", config.identifier, "Identifier of the dataset. Leads image file names");
        app.add_option("-p,--path", config.imagePath, "Path where generated images are stored");

        app.add_flag("--staticPositions", config.staticPositions, "Locks Object Position to specific tiles for specific counts. Persistent between test and val.");
        app.add_flag("--staticRotations", config.staticRotations, "Locks Object Rotation to always face one way.");
        app.add_flag("--noColor", config.noColor, "Turns off Object Colouring.");
        app.add_flag("--staticCam", config.staticCam, "Turns off Camera Rotation.");
        app.add_flag("--noExtras", config.noExtras, "No longer populates non active Object tiles.");
        app.add_flag("--noTileColors", config.noTileColors, "Turns of colors on floor tiles.");
        app.add_flag("--oldQuesSplit", config.oldSplit, "Sets the Ratio of questions to 1:2:2");
        app.add_flag("--noShader", config.noShader, "Emergency Compatibility, disables shaders entirely");
        CLI11_PARSE(app, argc, argv);
    }
    if (!config.verify()) {
        //Checks all values are set correctly, stops if not
        return -1;
    }
    config.printFile();
    //Prints config file
    if (config.extDebug) {
        osg::setNotifyLevel(osg::NotifySeverity::DEBUG_INFO);
    }

//Initialization
    RNGStream.seed(config.seed);
    cout << "Initializing Viewer, Vectors\n";
    osgViewer::Viewer viewer;

    //Initializes the basic camera and light rotaion vectors
    osg::Vec3 center(0,0,0);
    osg::Vec3 up(0,1,0);
    double distanceFromCenter = 2*(double)config.bsize*(double)config.stepsize; //Distance makes diagonal of chessboard fit almost exactly in frame
    osg::Vec3 baseEye(0, 0.5*distanceFromCenter, distanceFromCenter);
    osg::Vec3 eye = baseEye;
    osg::Vec4 eyeLight(0, 0.5 * distanceFromCenter, distanceFromCenter,1);

    //Creates the Pixelbuffer, which is used to write the  image to disk
    //Code taken from OSG-Examples autocapture.cpp
    cout << "Initializing Pixel buffer\n";
    osg::DisplaySettings *disSet = osg::DisplaySettings::instance().get();
    ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(disSet);
    cout << "Loading Camera Traits 2\n";
    //viewer would use fullscreen size (unknown here) pbuffer will use 4096 x4096 (or best available)
    traits->width = 4096;
    traits->height = 4096;
    traits->pbuffer = true;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();
    cout << "Loading PBuffer\n";
    ref_ptr<osg::GraphicsContext> pbuffer = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (config.extDebug) {
        pbuffer->getState()->setCheckForGLErrors(osg::State::CheckForGLErrors::ONCE_PER_ATTRIBUTE);
    }
    if(pbuffer.valid()){
        ref_ptr<osg::Camera> camera = new osg::Camera(*(viewer.getCamera()));
        camera->setGraphicsContext(pbuffer.get());
        camera->setViewport(new osg::Viewport(0,0,traits->width, traits->height));
        GLenum buffer = pbuffer->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        viewer.setCamera(camera.get());
    }
    else{
        cout << "pbuffer failed\n";
        return -1;
    }

    //Creates the Main scene, that holds all other objects    
    ref_ptr<osg::Group> scene = new osg::Group;
    scene->setDataVariance(osg::Object::DataVariance::DYNAMIC); //This is required so the board isn't allowed to get updated during the image printing process

    //Creates the Light Source
    cout << "Making Light\n";
    ref_ptr<osg::LightSource> lSrc = makeLight(config);

    //Initializes the Shadowed Scene and attaches it to the main scene
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene;
    if(!config.noShader){
        shadowedScene = makeShadow(lSrc, config);
        scene->addChild(shadowedScene.get());
    }
    else {
        scene->addChild(lSrc);
    }

//Generate Chessboard of translations
    std::cout << "Making Translations\n";
    vector<ref_ptr<MatrixTransform> > translations; //Holds all Nodes containing Object Positions
    vector<ref_ptr<MatrixTransform> > rotations;    //Holds all Nodes containing Object Rotations
    vector<ref_ptr<MatrixTransform> > floorTileLocs;//Holds all Nodes containing Tile Positions
    vector<ref_ptr<osg::Geode> > floorTiles;        //Holds all Floor Tiles 
    int tileCols;                                   //Holds the number of different tile colors
    std::tie(floorTiles, tileCols) = createFloorTiles(config);
    
    //Write Debug File
    osgDB::writeNodeFile(*floorTiles[0].get(), "DebugTile.osg");

    //Creates the Object Position, Object Rotation and Tile Position Graph Nodes
    if(config.noShader){
        std::tie(translations, rotations, floorTileLocs) = createTranslationsNoShadow(scene, floorTiles, config);
    }else{
        std::tie(translations, rotations, floorTileLocs) = createTranslations(shadowedScene, floorTiles, config);
    }

//Load Objects into memory
    std::cout << "Loading Objects\n";
    vector<vector<ref_ptr<osg::Group> > > objects; //Holds all object Nodes
    vector<string> objNames;                       //Holds all object name plurals
    vector<string> objSingular;                    //Holds all object name singulars
    int objCount;                                  //Number of objects
    //Loads the objects from their files, creates the coloured versions
    std::tie(objects, objNames, objSingular, objCount) = loadObjects(config);

//Initiate RNG
    std::cout << "Initializing RNG" << std::endl;
    std::uniform_int_distribution<int> objRand(0,objCount-1);                   //For randomly selecting an object
    std::uniform_int_distribution<int> locRand(0,config.bsize*config.bsize-1);  //For randomly selecting a tile on the board
    std::uniform_int_distribution<int> colRand(0,5);                            //For randomly selecting an object colour
    std::uniform_real_distribution<double> rotRand(0,2);                        //For randomly selecting a rotation radian
    std::uniform_int_distribution<int> floorRand(0, tileCols-1);                //For randomly selecting a floor tile colour
    
//Generate Sequence of Objects Existence/Non-Existence Questions
    int scenesPerObject; //Image is 3 times this with active Cam
    scenesPerObject = config.constellations * 11;
    vector<int> bannedObjects;  //Contains the id of the banned object for each possible image
    for (int i = 0; i < objCount; i++) {
        for (int j = 0; j < scenesPerObject; j++) {
            int bannedObj = objRand(RNGStream);
            while(bannedObj == i) {
                bannedObj = objRand(RNGStream);
            }
            bannedObjects.push_back(bannedObj);
        }
    }

//Final Initialization
    std::cout << "Create SceneHandler\n";
    SceneHandler control(objects, floorTiles,  translations, rotations, floorTileLocs, eyeLight, lSrc, config);
    control.setRNG(&RNGStream, &objRand, &locRand, &colRand, &rotRand, &floorRand); //Split from Constructor for legibility
    control.setStaticPos(); //Generates static positions if necessary
    std::cout << "Setting Change\n";
    control.setChange();//Loads the Flow-Control-Variables from the Config
    std::cout << "First Update\n";
    control.update(0, 0, 1); //First update
    osgDB::writeNodeFile(*scene.get(), "debugSceneDump.osg"); //Debug write to Disk (This takes quite long)

    //Initializes the Light and Camera Positions
    std::cout << "Set Light and Camera" << std::endl;
    //Sets the Light to the given Position and orients it to face the center
    lSrc->getLight()->setPosition(osg::Vec4(0, distanceFromCenter, -distanceFromCenter, 1));
    osg::Vec3f lightDir = osg::Vec3f(eyeLight.x() * (-1), eyeLight.y() * (-1), eyeLight.z() * (-1));
    lightDir.normalize();
    lSrc->getLight()->setDirection(lightDir);

    //Rotates the Camera on its original position by 45�
    Matrix rot;
    rot.makeRotate(0.25 * M_PI, osg::Vec3(0, 1, 0));
    viewer.getCamera()->setViewMatrixAsLookAt(rot.preMult(baseEye), center, up);

    //Finalizes and loads viewer
    std::cout << "Realize Viewer" << std::endl;
    viewer.realize();
    //viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    viewer.setSceneData(scene.get());
    control.update(0, 5, 1);
    viewer.frame();
    
//Open Question and Annotation File
    std::cout << "Open Files" << std::endl;
    WriteHandler outFiles(objSingular, objNames, config);
    outFiles.Init();
//Loop
    std::cout << "Begin Loop" << std::endl;
    int ProgressionSentinel = -1;
    for(int objId = 0; objId < objCount; objId++){
        //Loop over amounts
        for(int objAm = 0; objAm <= 10; objAm++){
            //Loop over constellation
            for(int conStep = 0; conStep < config.constellations; conStep++){
                //Increments the total number of constellations created by one
                ProgressionSentinel++;

                //Updates the SceneGraph: Object Position, Color, Rotation, Floor Tile Color, Light
                //Returns 3 objects that are present on the Board
                int pObj1, pObj2, pObj3;
                std::tie(pObj1, pObj2, pObj3) = control.update(objId, objAm, bannedObjects[ProgressionSentinel]);
                //Loop over cameraset
                for(int camPos = 0; camPos < 3; camPos++){
                    //Chance Camera position
                    if (!config.staticCam) {
                        //Rotate Camera around (0,1,0)
                        Matrix rotate;
                        double rotFac = rotRand(RNGStream);
                        rotate.makeRotate(M_PI * rotFac, osg::Vec3(0, 1, 0));
                        eye = rotate.preMult(eye);
                        viewer.getCamera()->setViewMatrixAsLookAt(eye, center, up);
                    }
                    else {
                        camPos = 3;
                    }
                    //Get image ID and filename
                    string imageID = config.identifier + leftpad(objId, 3) + leftpad(objAm, 2) + leftpad(conStep,2) + std::to_string(camPos);
                    string filename = config.imagePath + imageID + ".png";
                    //Initialize Draw Callback
                    GLenum buffer = viewer.getCamera()->getGraphicsContext()->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
                    //Draw Callback gets called
                    viewer.frame();
                    viewer.getCamera()->setFinalDrawCallback(new WindowCaptureCallback(buffer, filename));
                    viewer.frame();
                    viewer.getCamera()->setFinalDrawCallback(NULL);

                    //Set Flag to determine how often Are There and Is There questions are generated
                    bool extraQues = config.oldSplit;
                    if (camPos == 0) {
                        extraQues = true;
                    }
                    if (camPos == 3) {
                        if (conStep == 0) {
                            extraQues = true;
                        }
                    }
                    //write Question and Annotation
                    {
                        //How many are there
                        outFiles.QuesHM(objId, objAm, imageID);
                        outFiles.AnnoHM(objId, objAm, imageID);
                    }
                    if(extraQues){
                        //Are there X objects? Correct Answer
                        bool truth = true;
                        outFiles.QuesAT(objId, objAm, imageID, truth);
                        outFiles.AnnoAT(objId, objAm, imageID, truth);
                    }
                    if (extraQues) {
                        //Are there X objects? Displaced Answer
                        bool truth = false;
                        outFiles.QuesAT(objId, objAm, imageID, truth);
                        outFiles.AnnoAT(objId, objAm, imageID, truth);
                    }
                    if (extraQues && !config.noExtras) {
                        //Is there Object X - True
                        bool truth = true;
                        //Initialize Chosen Object with the first one
                        int presObj = pObj1;
                        //Override Chosen Object in Case Static Camera is turned on and Questions are generated for each Camera Position
                        switch (camPos) {
                        case 0:
                            presObj = pObj1;
                            break;
                        case 1:
                            presObj = pObj2;
                            break;
                        case 2:
                            presObj = pObj3;
                            break;
                        case 3:
                            break;
                        }
                        outFiles.QuesIT(presObj, imageID, truth);
                        outFiles.AnnoIT(presObj, imageID, truth);
                    }
                    if (extraQues && !config.noExtras) {
                        //Is there Object X - False
                        bool truth = false;
                        //Only one banned Object per Constellation
                        int bannedObj = bannedObjects[ProgressionSentinel];
                        outFiles.QuesIT(bannedObj, imageID, truth);
                        outFiles.AnnoIT(bannedObj, imageID, truth);
                    }
                }
            }
        }
    }
//End Steps
    outFiles.End();
    cout << "Complete.\n";
    return 0;
}


/*CreateExamplePictures.cpp
* Used to create the Object Example 
* Parameters used:
* Color: 3 Offset: 60
* Color:-1 Offset: 40
* Color: 2 Offset: 30
*/

#ifdef _WIN32
    #define _USE_MATH_DEFINES // for C++
    #include<windows.h>
#endif

#include<osg/ref_ptr>
#include<osgViewer/Viewer>
#include<osgDB/ReadFile>
#include<osgDB/WriteFile>
#include<osgShadow/ShadowedScene>
#include<osgShadow/ShadowMap>
#include<osg/StateSet>

#include"CLI11.hpp"
#include"Utils.hpp"

#include<iostream>
#include<string>
#include<vector>


using std::cout;
using std::string;
using std::vector;
using osg::ref_ptr;

int main(int argc, char* argv[]) {
    string target = "";
    int offset = 80;
    int color = -1;
    double rotation = 0;

    CLI::App app{ "Tool to generate datasets to test counting in VisualQA." };
    app.add_option("-t,--target", target, "Path to DemoObject.");
    app.add_option("-o", offset, "Size of object");
    app.add_option("-c", color, "Color for object");
    app.add_option("-r", rotation, "Rotate Object by this");


    CLI11_PARSE(app, argc, argv);

    if (target == "Unassigned") {
        cout << "Please specify models to load." << std::endl;
        return -1;
    }
    else {
        cout << "Target is " << target << "." << std::endl;
    }


    //Initializes Viewer and Pixelbuffer. Pixelbuffer Code taken from OSG-Example autocapture.cpp
    osgViewer::Viewer viewer;
    cout << "Initializing Pixel buffer\n";
    osg::DisplaySettings* disSet = osg::DisplaySettings::instance().get();
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
    if (pbuffer.valid()) {
        ref_ptr<osg::Camera> camera = new osg::Camera(*(viewer.getCamera()));
        camera->setGraphicsContext(pbuffer.get());
        camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
        GLenum buffer = pbuffer->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        viewer.setCamera(camera.get());
    }
    else {
        cout << "pbuffer failed\n";
    }

    //Creates the main scene and sets Data Variance
    ref_ptr<osg::Group> scene = new osg::Group;
    scene->setDataVariance(osg::Object::DataVariance::DYNAMIC);

    //Creates a Light Source identical to the one used for the Main Program
    cout << "Making Light\n";
    ref_ptr<osg::LightSource> lSrc = new osg::LightSource;
    ref_ptr<osg::Light> light = new osg::Light;
    {
        osg::Vec4 lpos = osg::Vec4(offset, offset, offset, 1);
        light->setLightNum(0);
        light->setAmbient(osg::Vec4(1, 1, 1, 1));
        light->setDiffuse(osg::Vec4(1, 1, 1, 1));
        light->setSpecular(osg::Vec4(0, 0, 0, 1));
        light->setPosition(lpos);
        light->setDirection(osg::Vec3(0, -1, 0));
        light->setConstantAttenuation(1);
        light->setLinearAttenuation(0);
        light->setQuadraticAttenuation(0);
        lSrc->setLight(light);
    }

    //Creates the ShadowedScene Node to implement shadow casting
    const int ReceivesShadowTraversalMask = 0x1;
    const int CastsShadowTraversalMask = 0x2;
    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
    osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
    {
        sm->setLight(lSrc);
        sm->setTextureSize(osg::Vec2s(4096, 4096));
        sm->setAmbientBias(osg::Vec2(0.8, 1 - 0.8));
        shadowedScene->setShadowTechnique(sm.get());
        shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
        shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
        shadowedScene->addChild(lSrc);
        scene->addChild(shadowedScene.get());
    }

    //Reads the designated Object and rotates it by the given value
    ref_ptr<osg::Node> object = osgDB::readNodeFile(target);
    vector<ref_ptr<osg::StateSet> > colors = makeColors();
    ref_ptr<osg::Group> colObj = new osg::Group();
    ref_ptr<osg::MatrixTransform> rotObj = new osg::MatrixTransform();
    osg::Matrix mat;
    mat.makeRotate(rotation, osg::Vec3(0, 1, 0));
    rotObj->setMatrix(mat);
    rotObj->addChild(object);
    colObj->addChild(rotObj);
    //If a color was set, make the object that color
    if (color != -1) {
        std::cout << "Setting Color";
        colObj->setStateSet(colors[color]);
    }

    //Create the tile underneath the object
    ref_ptr<osg::Geometry> tile = new osg::Geometry;
    double corners = (double)offset / 2;
    ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    v->push_back(osg::Vec3(corners, 0.f, corners));
    v->push_back(osg::Vec3(-corners, 0.f, corners));
    v->push_back(osg::Vec3(-corners, 0.f, -corners));
    v->push_back(osg::Vec3(corners, 0.f, -corners));
    tile->setVertexArray(v.get());

    ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
    normal->push_back(osg::Vec3(0, 1.f, 0));
    tile->setNormalArray(normal);
    tile->setNormalBinding(osg::Geometry::BIND_OVERALL);
    tile->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    ref_ptr<osg::Geode> floorTile = new osg::Geode;
    floorTile->setNodeMask(~CastsShadowTraversalMask);
    floorTile->setStateSet(colors[0]);
    floorTile->addDrawable(tile);

    //Add Tile and Object to the scene
    shadowedScene->addChild(colObj);
    shadowedScene->addChild(floorTile);

    //Initialize Scene
    viewer.setSceneData(shadowedScene);
    viewer.realize();
    viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3(0, offset, 2 * offset), osg::Vec3(0, 0, 0), osg::Vec3(0, 1, 0));

    //Capture Image
    viewer.frame();
    GLenum buffer = viewer.getCamera()->getGraphicsContext()->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
    string filename = "ForPaper" + std::to_string(color) + std::to_string(offset) + ".png";
    viewer.frame();
    viewer.getCamera()->setFinalDrawCallback(new WindowCaptureCallback(buffer, filename));
    viewer.frame();
    viewer.getCamera()->setFinalDrawCallback(NULL);






}
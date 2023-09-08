/*UpdateStep.cpp
* Defines the SceneHandlers and WriteHandler classes
*/
#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif

#include"UpdateStep.h"

SceneHandler::SceneHandler(vector<vector<ref_ptr<osg::Group> > > objectsIn, vector<ref_ptr<osg::Geode> > floorTilesIn, MatTransVec translationsIn, MatTransVec rotationsIn, MatTransVec floorTileLocsIn, osg::Vec4 lightPosIn, ref_ptr<osg::LightSource> lSrcIn, Config configIn) {
    objects = objectsIn;
    floorTiles = floorTilesIn;
    translations = translationsIn;
    rotations = rotationsIn;
    floorTileLocs = floorTileLocsIn;
    rngStream = NULL;
    objRand = NULL;
    locRand = NULL;
    colRand = NULL;
    rotRand = NULL;
    firstPass = configIn.extDebug;
    config = configIn;
    lSrc = lSrcIn;
    eyeLight = lightPosIn;


}

void SceneHandler::setRNG(std::mt19937* rngStreamIn, std::uniform_int_distribution<int>* objRandIn, std::uniform_int_distribution<int>* locRandIn, std::uniform_int_distribution<int>* colRandIn, std::uniform_real_distribution<double>* rotRandIn, std::uniform_int_distribution<int>* floorRandIn) {
    rngStream = rngStreamIn;
    objRand = objRandIn;
    locRand = locRandIn;
    colRand = colRandIn;
    rotRand = rotRandIn;
    floorRand = floorRandIn;
}

void SceneHandler::setStaticPos() {
    //trigger only if Static Positions are used
    if (config.staticPositions) {
        staticPositions;  //For each Object amount holds a vector
                                                //with an entry for each board position holding a bool whether there's a counted object or not
        //Create RNG over all possible Board Positions
        std::uniform_int_distribution<int> staticPosRand(0, (config.bsize*config.bsize) - 1);
        //Create RNG stream from static seed
        std::mt19937 staticPosRNGStream;
        staticPosRNGStream.seed(config.staticSeed);

        //For all 11 possible Object amounts, a vector is required
        for (int i = 0; i <= 10; i++) {
            vector<bool> currentStatic; //Will hold the values for each board position
            for (int j = 0; j < (config.bsize*config.bsize); j++) {
                //initialize to board size with no objects
                currentStatic.push_back(false);
            }
            for (int j = 0; j < i; j++) {
                //Generate random position
                int currRand = staticPosRand(staticPosRNGStream);
                while (currentStatic[currRand]) {
                    //While the position has already been chosen, regenerate. Highest odds is 9/49 meaning should never have looping issues
                    currRand = staticPosRand(staticPosRNGStream);
                }
                currentStatic[currRand] = true;
            }
            //Save vector
            staticPositions.push_back(currentStatic);
        }
    }
}

void SceneHandler::setChange() {
    //Some are flipped for readability of update step
    colorChange = !config.noColor;
    rotationChange = !config.staticRotations;
    positionChange = !config.staticPositions;
    noExtras = config.noExtras;
    floorColors = !config.noTileColors;
}

std::tuple<int, int, int> SceneHandler::update(int objectID, int objectCount, int bannedObj){

    /*
    objects layer 0 - object type
            layer 1 - colour
    translations chessboard, chained row by row
    objectID - ID of active object
    objectcount - number of times this object needs to be spawned
    rngstream - for reproducability
    objRand, locRand, colRand - distributions to randomize the position/colour/other objects
    */

    vector<int> loadKey;    //Will hold an entry for each board tile, designating the object there
    vector<int> colorKey;   //Will hold an entry for each board tile, designating the object color there
//Generate Colour and Object Key
    bool firstLoop = false; //For Debug Output
    if (firstPass) {
        firstLoop = true;
        std::cout << "Generating Translation Associations" << std::endl;
    }
    for (int i = 0; i < translations.size(); i++) {
        //Generate a correspondence key from each chessboard tile to a random object except the active one
        int id;
        if (noExtras) {
            //While extras are disabled, fill loadKey with -1
            if (firstLoop) {
                std::cout << "no Extras" << std::endl;
            }
            id = -1;
        }
        else {
            //Generate Random Object
            id = objRand->operator()(*(rngStream));
            while (id == objectID || id == bannedObj) {
                //While Random object is either the one being counted or the banned one, regenerate
                id = objRand->operator()(*(rngStream));
            }
            if (firstPass) {
                std::cout << "Random Object: " << id << std::endl;
            }
        }
        //Save to vector
        loadKey.push_back(id);
        //Generate a correspondence key from each chessboard tile to a colour
        if (colorChange) {
            //If Object colours are activated, fill colorKey with random colour ids, shifted by 1
            if (firstLoop) {
                std::cout << "Set Colour" << std::endl;
                firstLoop = false;
            }
            int cl = colRand->operator()(*(rngStream));
            colorKey.push_back(cl + 1);
        }
        else {
            //Else fill with 0
            if (firstLoop) {
                std::cout << "No Colour" << std::endl;
                firstLoop = false;
            }
            colorKey.push_back(0);
        }
    }
//Change Key to involve specific object
    if (firstPass) std::cout << "Setting Disgnated Object" << std::endl;
    if (positionChange) {
        for (int i = 0; i < objectCount; i++) {
            //Replace random tile associations with the object, up to count
            int loc = locRand->operator()(*(rngStream));
            while (loadKey[loc] == objectID) { //Skips replacing active object with active object
                loc = locRand->operator()(*(rngStream));
            }
            loadKey[loc] = objectID;
        }
    }
    else {
        //If changing positions is deactivated, instead read the positions from the saved static vector
        vector<bool> actVector = staticPositions[objectCount];
        for (int i = 0; i < translations.size(); i++) {
            if (actVector[i]) {
                loadKey[i] = objectID;
            }
        }

    }
//Swap out all Objects
    if (firstPass) std::cout << "Updating Scene" << std::endl;
    for (int i = 0; i < translations.size(); i++) {
        //Remove all Children
        int childCount = translations[i]->getNumChildren();
        if (childCount != 0) {
            translations[i]->removeChildren(0, childCount);
        }
        //If Object ID is not -1, load Object in given Color
        if (loadKey[i] != -1) {
            translations[i]->addChild(objects[loadKey[i]][colorKey[i]].get());
        }
    }

    //Rotate All Objects randomly if not disabled
    if (firstPass) std::cout << "Rotating Scene" << std::endl;
    if (rotationChange) {
        for (int i = 0; i < rotations.size(); i++) {
            //Generate rotation radians
            double rotFac = rotRand->operator()(*(rngStream));

            //First we need to center the object on 0, so we can rotate around (0,1,0)
            osg::Matrix trans = translations[i]->getMatrix();
            osg::Vec3d transVec = trans.getTrans();
            osg::Matrix untrans;
            untrans.makeTranslate(-transVec);
            //Generate Rotation Matrix
            osg::Matrix rot;
            osg::Matrix joint;
            rot.makeRotate(rotFac * M_PI, osg::Vec3(0, 1, 0));
            //Center->Rotate->Uncenter
            joint = untrans * rot * trans;
            rotations[i]->setMatrix(joint);
        }
    }

    //Get three random Objects present on the board For passing for Question generation
    int objOne = locRand->operator()(*rngStream);
    int objTwo = locRand->operator()(*rngStream);
    int objThree = locRand->operator()(*rngStream);
    objOne = loadKey[objOne];
    objTwo = loadKey[objTwo];
    objThree = loadKey[objThree];

    //Randomly Color Floor Tiles
    vector<int> floorKey;
    if (floorColors) {
        for (int i = 0; i < floorTileLocs.size(); i++) {
            //Unattach Floor Tiles
            floorTileLocs[i]->removeChildren(0, floorTileLocs[i]->getNumChildren());
            //Attach new Floor Tiles
            int colId = floorRand->operator()(*(rngStream));
            floorTileLocs[i]->addChild(floorTiles[colId].get());
        }
    }

    //Update Lighting
    //Generate rotation radian around the y-Axis
    osg::Matrix rotateLight;
    double rotFacLight = rotRand->operator()(*(rngStream));
    rotateLight.makeRotate(M_PI* rotFacLight, osg::Vec3(0, 1, 0));
    //Generate new position
    eyeLight = rotateLight.preMult(eyeLight);
    //Set Light
    lSrc->getLight()->setPosition(eyeLight);
    //Reorient to face the Origin
    osg::Vec3f lightDir = osg::Vec3f(eyeLight.x() * (-1), eyeLight.y() * (-1), eyeLight.z() * (-1));
    lightDir.normalize();
    lSrc->getLight()->setDirection(lightDir);

    firstPass = false;
    return std::tuple<int, int, int>(objOne, objTwo, objThree);
}

WriteHandler::WriteHandler(vector<string> objNamesS, vector<string> objNamesP, Config configIn) {
    objNamesSingular = objNamesS;
    objNamesPlural = objNamesP;
    first = true;
    quesID = 0;
    config = configIn;

}

bool WriteHandler::Init() {
    //Open Streams
    string qFName;
    string aFName;
    if (config.train) {
        qFName = "OSGSet_" + config.identifier + "_train_ques.json";
        aFName = "OSGSet_" + config.identifier + "_train_anno.json";
    }
    else {
        qFName = "OSGSet_" + config.identifier + "_val_ques.json";
        aFName = "OSGSet_" + config.identifier + "_val_anno.json";
    }
    quesOut.open(qFName);
    annoOut.open(aFName);
    //Writes Initial Content
    InitQues();
    InitAnno();
    first = true;
    return true;
}

bool WriteHandler::InitAnno(){
    string assembly = "{\"info\": ";
    string info = "{\"year\": 2021, \"version\": \"Not Applicable\",";
    info = info + "\"description\": \"Dataset for Counting generated with OSG\",";
    info = info + "\"contributor\": \"Moritz Vannahme\",";
    info = info + "\"url\": \"Not Applicable\",";
#ifdef _WIN32
    char date[19];
    std::time_t t = time(0);
    struct tm timestamp;
    //Time code is weird
    ::localtime_s(&timestamp, &t);
    std::strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timestamp);
#else
    string date = "98-01-01 00:00:00";
#endif
    info = info + "\"date_created\": \"" + date + "\"";
    info = info + "},";
    assembly = assembly + info + "\"data_type\": \"OSGCreated\",";
    if (config.train) {
        assembly = assembly + "\"data_subtype\": \"train\",";
    }
    else {
        assembly = assembly + "\"data_subtype\": \"val\",";
    }
    assembly = assembly + "\"annotations\":[\n";
    annoOut << assembly;
    return true;
}

bool WriteHandler::InitQues(){
    string assembly = "{\"info\": ";
    string info = "{\"year\": 2021, \"version\": \"Not Applicable\",";
    info = info + "\"description\": \"Dataset for Counting generated with OSG\",";
    info = info + "\"contributor\": \"Moritz Vannahme\",";
    info = info + "\"url\": \"Not Applicable\",";
#ifdef _WIN32
    char date[19];
    std::time_t t = time(0);
    struct tm timestamp;
    ::localtime_s(&timestamp, &t);
    std::strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &timestamp);
#else
    string date = "98-01-01 00:00:00";
#endif
    info = info + "\"date_created\": \"" + date + "\"";
    info = info + "},";
    assembly = assembly + info;
    assembly = assembly + "\"task_type\": \"OpenEnded\",";
    assembly = assembly + "\"data_type\": \"OSGCreated\",";
    if (config.train) {
        assembly = assembly + "\"data_subtype\": \"train\",";
    }
    else {
        assembly = assembly + "\"data_subtype\": \"val\",";
    }
    assembly = assembly + "\"questions\":[\n";
    quesOut << assembly;
    return true;
}

bool WriteHandler::QuesHM(int objID, int objAmount, string imageID) {
    quesID++;   //Increment Question ID only on question
    if (!first) { quesOut << ","; } //Suppresses a comma on the very first entry
    string assembly = "{\"question_id\": ";
    assembly = assembly + std::to_string(quesID);
    assembly = assembly + ", \"image_id\": " + imageID;
    string question = "\"How many " + objNamesPlural[objID] + " are in the picture?\"";
    assembly = assembly + ", \"question\": " + question + "}\n";
    quesOut << assembly;
    return true;
}

bool WriteHandler::AnnoHM(int objID, int objAmount, string imageID) {
    if (!first) { annoOut << ","; } //Suppresses a leading comma on the very first entry
    string assembly = "{\"question_id\": ";
    assembly = assembly + std::to_string(quesID);
    assembly = assembly + ", \"image_id\": " + imageID;
    assembly = assembly + ", \"question_type\": \"how many\"";
    assembly = assembly + ", \"answer_type\": \"number\"";
    assembly = assembly + ", \"answers\": [";
    for (int i = 1; i < 11; i++) {
        string answer = "";
        if (i != 1) { answer = ","; }
        answer = answer + "{\"answer_id\": " + std::to_string(i);
        answer = answer + ", \"answer\": \"" + std::to_string(objAmount) + "\"";
        answer = answer + ", \"answer_confidence\": \"yes\"}";
        assembly = assembly + answer;
    }
    assembly = assembly + "], \"multiple_choice_answer\": ";
    assembly = assembly + "\"" + std::to_string(objAmount) + "\"}\n";
    annoOut << assembly;
    first = false;  //Disable first pass only on annotation
    return true;
}

bool WriteHandler::QuesAT(int objID, int objAmount, string imageID, bool truth) {
    quesID++;   //Increment Question ID only on question
    if (!first) { quesOut << ","; } //Suppresses a comma on the very first entry
    string assembly = "{\"question_id\": ";
    assembly = assembly + std::to_string(quesID);
    assembly = assembly + ", \"image_id\": " + imageID;
    if (!truth) {
        if (objAmount < 5) {
            objAmount += 11;
        }
        objAmount -= 5;
    }
    string question = "\"Are there " + std::to_string(objAmount) + " " + objNamesPlural[objID] + " in the picture?\"";
    assembly = assembly + ", \"question\": " + question + "}\n";
    quesOut << assembly;
    return true;
}

bool WriteHandler::AnnoAT(int objID, int objAmount, string imageID, bool truth) {
    if (!first) { annoOut << ","; } //Suppresses a leading comma on the very first entry
    string assembly = "{\"question_id\": ";
    assembly = assembly + std::to_string(quesID);
    assembly = assembly + ", \"image_id\": " + imageID;
    assembly = assembly + ", \"question_type\": \"Are there\"";
    assembly = assembly + ", \"answer_type\": \"yes/no\"";
    assembly = assembly + ", \"answers\": [";
    for (int i = 1; i < 11; i++) {
        string answer = "";
        if (i != 1) { answer = ","; }
        answer = answer + "{\"answer_id\": " + std::to_string(i);
        if (truth) {
            answer = answer + ", \"answer\": \"yes\"";
        }
        else {
            answer = answer + ", \"answer\": \"no\"";
        }
        answer = answer + ", \"answer_confidence\": \"yes\"}";
        assembly = assembly + answer;
    }
    assembly = assembly + "], \"multiple_choice_answer\": ";
    if (truth) {
        assembly = assembly + "\"yes\"}\n";
    }
    else {
        assembly = assembly + "\"no\"}\n";
    }
    annoOut << assembly;
    first = false;  //Disable first pass only on annotation
    return true;
}

bool WriteHandler::QuesIT(int objID, string imageID, bool truth) {
    quesID++;   //Increment Question ID only on question
    if (!first) { quesOut << ","; } //Suppresses a comma on the very first entry
    string assembly = "{\"question_id\": ";
    assembly = assembly + std::to_string(quesID);
    assembly = assembly + ", \"image_id\": " + imageID;
    string question = "\"Is there " + objNamesSingular[objID] + " in the picture?\"";
    assembly = assembly + ", \"question\": " + question + "}\n";
    quesOut << assembly;
    quesOut.flush(); //Flush stream to file every so often
    return true;
}

bool WriteHandler::AnnoIT(int objID, string imageID, bool truth) {
    if (!first) { annoOut << ","; } //Suppresses a leading comma on the very first entry
    string assembly = "{\"question_id\": ";
    assembly = assembly + std::to_string(quesID);
    assembly = assembly + ", \"image_id\": " + imageID;
    assembly = assembly + ", \"question_type\": \"Is there\"";
    assembly = assembly + ", \"answer_type\": \"yes/no\"";
    assembly = assembly + ", \"answers\": [";
    for (int i = 1; i < 11; i++) {
        string answer = "";
        if (i != 1) { answer = ","; }
        answer = answer + "{\"answer_id\": " + std::to_string(i);
        if (truth) {
            answer = answer + ", \"answer\": \"yes\"";
        }
        else {
            answer = answer + ", \"answer\": \"no\"";
        }
        answer = answer + ", \"answer_confidence\": \"yes\"}";
        assembly = assembly + answer;
    }
    assembly = assembly + "], \"multiple_choice_answer\": ";
    if (truth) {
        assembly = assembly + "\"yes\"}\n";
    }
    else {
        assembly = assembly + "\"no\"}\n";
    }
    annoOut << assembly;
    annoOut.flush();//Flush Stream to file every so often
    first = false;  //Disable first pass only on annotation
    return true;
}

bool WriteHandler::End() {
    EndQues();//Writes end Content
    EndAnno();
    quesOut.close();//Close streams
    annoOut.close();
    return true;
}

bool WriteHandler::EndQues() {
    string assembly;
    assembly = "], \"license\": {";
    string license = "\"license\": \"Not Applicable\"";
    assembly = assembly + license + "}}";
    quesOut << assembly;
    return true;
}

bool WriteHandler::EndAnno() {
    string assembly;
    assembly = "], \"license\": {";
    string license = "\"license\": \"Not Applicable\"";
    assembly = assembly + license + "}}";
    annoOut << assembly;
    return true;
}
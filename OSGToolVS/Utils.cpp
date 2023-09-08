#pragma once
#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif

#include"Utils.hpp"



vector<ref_ptr<osg::StateSet> > makeColors() {
    
    vector<ref_ptr<osg::StateSet> > output;

    //Create a Material and set the Light Properties
    ref_ptr<osg::Material> materialRed = new osg::Material;
    materialRed->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 0, 0, 1));
    materialRed->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 0, 0, 1));
    materialRed->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3, 0.3, 0.3, 1));
    materialRed->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
    materialRed->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);

    //Make the Stateset in Override mode
    ref_ptr<osg::StateSet> stateRed = new osg::StateSet;
    stateRed->setAttribute(materialRed, osg::StateAttribute::OVERRIDE);

    //Add to Vector
    output.push_back(stateRed.get());

    ref_ptr<osg::Material> materialGreen = new osg::Material;
    materialGreen->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.5, 1, 0, 1));
    materialGreen->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.5, 1, 0, 1));
    materialGreen->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3, 0.3, 0.3, 1));
    materialGreen->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
    materialGreen->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateGreen = new osg::StateSet;
    stateGreen->setAttribute(materialGreen, osg::StateAttribute::OVERRIDE);
    output.push_back(stateGreen.get());

    ref_ptr<osg::Material> materialBlue = new osg::Material;
    materialBlue->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 1, 1));
    materialBlue->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 1, 1));
    materialBlue->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3, 0.3, 0.3, 1));
    materialBlue->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
    materialBlue->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateBlue = new osg::StateSet;
    stateBlue->setAttribute(materialBlue, osg::StateAttribute::OVERRIDE);
    output.push_back(stateBlue.get());

    ref_ptr<osg::Material> materialPurple = new osg::Material;
    materialPurple->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 0, 1, 1));
    materialPurple->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 0, 1, 1));
    materialPurple->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3, 0.3, 0.3, 1));
    materialPurple->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
    materialPurple->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> statePurple = new osg::StateSet;
    statePurple->setAttribute(materialPurple, osg::StateAttribute::OVERRIDE);
    output.push_back(statePurple.get());

    ref_ptr<osg::Material> materialWhite = new osg::Material;
    materialWhite->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
    materialWhite->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
    materialWhite->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3, 0.3, 0.3, 1));
    materialWhite->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
    materialWhite->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateWhite = new osg::StateSet;
    stateWhite->setAttribute(materialWhite, osg::StateAttribute::OVERRIDE);
    output.push_back(stateWhite.get());

    ref_ptr<osg::Material> materialBlack = new osg::Material;
    materialBlack->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.5, 0.5, 0.5, 1));
    materialBlack->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.5, 0.5, 0.5, 1));
    materialBlack->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3, 0.3, 0.3, 1));
    materialBlack->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
    materialBlack->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateBlack = new osg::StateSet;
    stateBlack->setAttribute(materialBlack, osg::StateAttribute::OVERRIDE);
    output.push_back(stateBlack.get());

    return output;
}

string leftpad(int input, int padTo) {
    //Leftpad with 0s
    string output = std::to_string(input);
    if (padTo < output.size()) {
        cout << "Error, input string is already longer than given value." << std::endl;
        cout << input << std::endl;
        throw "Exception";
    }
    while (padTo > output.size()) {
        output = "0" + output;
    }
    return output;
}

Config::Config() {
    target = "Unassigned";
    tileColors = "Unassigned";
    train = true;
    val = false;
    staticPositions = false;
    staticRotations = false;
    noColor = true;
    noExtras = false;
    staticCam = false;
    noTileColors = false;
    oldSplit = false;

    seed = 27;
    staticSeed = 27;

    constellations = 3;
    bsize = 7;
    stepsize = 40;

    identifier = "Unassigned";
    imagePath = "./Images/";
    
    aBias = 0.8; //Ambient Bias
    aBright = 1; //Ambient Light Brightness factor
    extDebug = false;
}

bool Config::verify() {
    if (target == "Unassigned") {
        //Check if Models given
        cout << "Please specify models to load." << std::endl;
        return false;
    }
    else {
        cout << "Target is " << target << "." << std::endl;
    }
    if (tileColors == "Unassigned" && !noTileColors) {
        //Check if Tile Colors required and given
        cout << "Please provide a file listing colours for the tiles." << std::endl;
        return false;
    }
    else if (noTileColors) {
        cout << "Loading no tile colors." << std::endl << std::endl;
    }
    else {
        cout << "Loading tile colors from " << tileColors << std::endl;
    }
    staticSeed = seed;
    if (val) {
        //Switch on changed seed if in validation mode
        train = false;
        cout << "Making a Validation Set." << std::endl;
        seed = 100344 - seed;
    }
    else {
        cout << "Making a Test Set." << std::endl;
    }
    cout << "Seed is " << seed << std::endl;
    cout << "Static Seed is " << staticSeed << " Checksum: " << staticSeed + seed - 100344 << std::endl << std::endl;
    cout << "Number of constellations per object amount is " << constellations << std::endl;
    cout << "Boardsize is " << bsize << " by " << bsize << std::endl;
    cout << "Stepsize is " << stepsize << std::endl << std::endl;
    if (identifier == "Unassigned") {
        cout << "Please specify an identifier." << std::endl;
        return -1;
    }
    else {
        cout << "Dataset being generated is " << identifier << std::endl;
    }
    cout << "Images stored at " << imagePath << std::endl;

    return true;
}

bool Config::printFile() {
    std::ofstream config;
    config.open("./Config.txt");
    if (val) { config << "Mode: Validation" << std::endl; }
    else { config << "Mode: Training" << std::endl; }
    config << std::endl << "Parameters" << std::endl;
    config << "Seed: " << seed << std::endl;
    config << "Static Seed: " << staticSeed << std::endl;
    config << "Constellations: " << constellations << std::endl;
    config << "Boardsize: " << bsize << std::endl;
    config << "Stepsize/Offset: " << stepsize << std::endl;
    config << "Identifier: " << identifier << std::endl;
    config << "Ambient Bias: " << aBias << std::endl;
    config << "Ambient Brightness: " << aBright << std::endl;

    config << std::endl << "Sources" << std::endl;
    config << "Model Source: " << target << std::endl;
    config << "Floor Tile Color Source: " << tileColors << std::endl;
    config << "Image Location: " << imagePath << std::endl;

    config << std::endl << "Flags" << std::endl;
    config << "Static Positions: " << staticPositions << std::endl;
    config << "Static Rotations: " << staticRotations << std::endl;
    config << "No Color: " << noColor << std::endl;
    config << "Static Cam: " << staticCam << std::endl;
    config << "No Extras: " << noExtras << std::endl;
    config << "No Tile Colors: " << noTileColors << std::endl;
    config.close();
    return true;
}
/*Testing.cpp
* Displays the Object placed on a regular board, to check if placing on floor worked
* Alternatively (currently commented out )reads the debug variant of the Object Conversion file
* and loads it to display all Conversion steps coloured.
* Used to bugfix the conversion. Currently not needed
*/

#pragma once
#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif
#include<osg/ref_ptr>
#include<osg/Node>
#include<osg/Geometry>
#include<osg/Geode>
#include<osg/Group>
#include<osg/MatrixTransform>
#include<osgDB/ReadFile>
#include<osgDB/WriteFile>
#include<osg/Material>
#include<osgViewer/Viewer>
#include<osgGA/TrackballManipulator>
#include<osgShadow/ShadowedScene>
#include<osgShadow/ShadowMap>
#include<osgShadow/ShadowTexture>

#include<string>
#include<vector>
#include<iostream>
#include"CLI11.hpp"

using std::cout;
using std::string;
using std::vector;

using osg::ref_ptr;
using osg::Node;
using osg::Matrix;
using osg::MatrixTransform;
using osgViewer::Viewer;

vector<ref_ptr<osg::StateSet> > makeColors();

int main(int argc, char *argv[]){

    string target = "";
    CLI::App app{ "Pre-Converts 3DS-Models to OSG-Format." };
    app.add_option("-t,--target", target, "File containing the objects to load.");
    CLI11_PARSE(app, argc, argv);

    if (target == "") {
        cout << "Please specify models to load.\n";
    }
    else {
        cout << "Target is " << target << "\n";
    }

    ref_ptr<Node> openedNode = osgDB::readNodeFile("./Converted/"+target+".osg");
    /*ref_ptr<Node> debug1 = osgDB::readNodeFile("./Debug/" + target + "1.osg");
    ref_ptr<Node> debug2 = osgDB::readNodeFile("./Debug/"+target+"2.osg");
    ref_ptr<Node> debug3 = osgDB::readNodeFile("./Debug/"+target+"3.osg");
    ref_ptr<Node> debugF = osgDB::readNodeFile("./Debug/" + target + "F.osg");*/
    vector<ref_ptr<osg::StateSet>> chroma = makeColors();
    /*debug1->setStateSet(chroma[0].get());
    debug2->setStateSet(chroma[2].get());
    debug3->setStateSet(chroma[1].get());*/
    openedNode->setStateSet(chroma[3].get());
    //Spawn scene
    ref_ptr<osgShadow::ShadowedScene> scene = new osgShadow::ShadowedScene;
    ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;
    int CastsShadowTraversalMask = 0x2;
    scene->setCastsShadowTraversalMask(CastsShadowTraversalMask);

    scene->setShadowTechnique(sm.get());

    int mapres = 1024;
    sm->setTextureSize(osg::Vec2s(mapres, mapres));
    ref_ptr<osg::LightSource> lSrc = new osg::LightSource;
    ref_ptr<osg::Light> light = new osg::Light;
    osg::Vec4 lpos = osg::Vec4(100,-100,100,1);
    light->setAmbient(osg::Vec4(0,0,0,0));
    light->setDiffuse(osg::Vec4(0.8,0.8,0.8,0));
    light->setSpecular(osg::Vec4(0,0,0,0));
    light->setPosition(lpos);
    light->setDirection(osg::Vec3(-1,-1,-1));
    lSrc->setLight(light);
    sm->setLight(lSrc);
    scene->addChild(lSrc);
    


    vector<ref_ptr<MatrixTransform> > translations;
    //Tile x = a, z = b is at (b*bsize + a)
    //For real coordinates of (b*bsize + a) -> x = stepsize*(a-bOffset), z = stepsize*(b-bOffset)
    //0 - (bsize-1)         first z-Axis-row
    //bsize - (2bsize - 1)  second z-Axis-row and so forth
    ref_ptr<osg::Geode> floorTileWhite = new osg::Geode;
    ref_ptr<osg::Geode> floorTileOrange = new osg::Geode;
    //Define Floor Tiles
    {
        cout << "Making Floor Tiles\n";
        ref_ptr<osg::Material> White = new osg::Material;
        White->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.8, 0.8, 0.8, 0));
        White->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 1, 1, 0));
        White->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 0));
        White->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 0));
        White->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
        ref_ptr<osg::StateSet> stateWhite = new osg::StateSet;
        stateWhite->setAttribute(White, osg::StateAttribute::OVERRIDE);
        floorTileWhite->setStateSet(stateWhite);
        ref_ptr<osg::Material> Orange = new osg::Material;
        Orange->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.8, 0.4, 0, 1));
        Orange->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 0.5, 0, 1));
        Orange->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
        Orange->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
        Orange->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
        ref_ptr<osg::StateSet> stateOrange = new osg::StateSet;
        stateOrange->setAttribute(Orange, osg::StateAttribute::OVERRIDE);
        floorTileOrange->setStateSet(stateOrange);

        ref_ptr<osg::Geometry> tile = new osg::Geometry;
        double corners = 1;
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
        floorTileWhite->addDrawable(tile);
        floorTileOrange->addDrawable(tile);
    }
    bool flip = true;
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            osg::Matrix translate;
            ref_ptr<osg::MatrixTransform> transfer = new osg::MatrixTransform;
            translate.setTrans((-5 + i) * 2, 0, (-5 + j) * 2);
            transfer->setMatrix(translate);
            if (flip) {
                transfer->addChild(floorTileWhite);
                flip = false;
            }
            else {
                transfer->addChild(floorTileOrange);
                flip = true;
            }
            scene->addChild(transfer);
        }
    }


    //Load a model
    cout << "Model attached\n";
    ref_ptr <MatrixTransform > displace = new MatrixTransform;
    Matrix dismat;
    dismat.makeTranslate(-10, 0, -10);
    displace->setMatrix(dismat);
    displace->addChild(openedNode);
    scene->addChild(openedNode);
    scene->addChild(displace);
    /*scene->addChild(debug1);
    scene->addChild(debug2);
    scene->addChild(debug3);
    scene->addChild(debugF);*/

    //show scene
    cout << "File written\n";
    cout.flush();
    Viewer display;
    display.setSceneData(scene.get());
    display.setCameraManipulator(new osgGA::TrackballManipulator);
    //std::cout << display.run();
    display.realize();
    int i = 0;
    int j = 0;
    while(!display.done()){
        display.frame();
    }
}


vector<ref_ptr<osg::StateSet> > makeColors(){
    vector<ref_ptr<osg::StateSet> > output;
    ref_ptr<osg::Material> materialRed = new osg::Material;
    materialRed->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1,0,0,1));
    materialRed->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1,0,0,1));
    materialRed->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.3,0.3,1));
    materialRed->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    materialRed->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateRed = new osg::StateSet;
    stateRed->setAttribute(materialRed, osg::StateAttribute::OVERRIDE);
    output.push_back(stateRed.get());
    
    ref_ptr<osg::Material> materialGreen = new osg::Material;
    materialGreen->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.6,0,1));
    materialGreen->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.6,0,1));
    materialGreen->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.3,0.3,1));
    materialGreen->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    materialGreen->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateGreen = new osg::StateSet;
    stateGreen->setAttribute(materialGreen, osg::StateAttribute::OVERRIDE);
    output.push_back(stateGreen.get());

    ref_ptr<osg::Material> materialBlue = new osg::Material;
    materialBlue->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,1,1));
    materialBlue->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,1,1));
    materialBlue->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.3,0.3,1));
    materialBlue->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    materialBlue->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateBlue = new osg::StateSet;
    stateBlue->setAttribute(materialBlue, osg::StateAttribute::OVERRIDE);
    output.push_back(stateBlue.get());
    
    ref_ptr<osg::Material> materialPurple = new osg::Material;
    materialPurple->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.8,0,0.8,1));
    materialPurple->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1,0,1,1));
    materialPurple->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.3,0.3,1));
    materialPurple->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    materialPurple->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> statePurple = new osg::StateSet;
    statePurple->setAttribute(materialPurple, osg::StateAttribute::OVERRIDE);
    output.push_back(statePurple.get());
    
    ref_ptr<osg::Material> materialWhite = new osg::Material;
    materialWhite->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.8,0.8,0.8,1));
    materialWhite->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1,1,1,1));
    materialWhite->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.3,0.3,1));
    materialWhite->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    materialWhite->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateWhite = new osg::StateSet;
    stateWhite->setAttribute(materialWhite, osg::StateAttribute::OVERRIDE);
    output.push_back(stateWhite.get());

    ref_ptr<osg::Material> materialBlack = new osg::Material;
    materialBlack->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.01,0.01,0.01,1));
    materialBlack->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.01,0.01,0.01,1));
    materialBlack->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0.3,0.3,0.3,1));
    materialBlack->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    materialBlack->setShininess(osg::Material::Face::FRONT_AND_BACK, 10);
    ref_ptr<osg::StateSet> stateBlack = new osg::StateSet;
    stateBlack->setAttribute(materialBlack, osg::StateAttribute::OVERRIDE);
    output.push_back(stateBlack.get());

    return output;
}

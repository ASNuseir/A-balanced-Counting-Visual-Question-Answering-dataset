/*Init.h
* Contains Initializer functions for various SceneGraph Objects
*/

#pragma once
#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif
#include<osgShadow/ShadowedScene>
#include<osgShadow/ShadowMap>
#include<osg/ref_ptr>
#include<osg/MatrixTransform>
#include<osg/Geode>
#include<osg/Material>
#include<osg/StateSet>
#include<osg/Geometry>
#include<osg/LightSource>
#include<osgDB/ReadFile>

#include<string>
#include<vector>
#include<chrono>
#include<ctime>
#include<string>
#include<iostream>
#include<fstream>

#include"Utils.hpp"

using std::string;
using std::vector;
using std::cout;

using osg::ref_ptr;
using osg::MatrixTransform;
using osg::Matrix;
using osg::Node;

using MatTransVec = std::vector<osg::ref_ptr<osg::MatrixTransform> >;

std::tuple < MatTransVec, MatTransVec, MatTransVec> createTranslations(ref_ptr<osgShadow::ShadowedScene> scene, vector<ref_ptr<osg::Geode> > floorTiles, Config config);
/*Creates and Returns all Floor Tile Positions, Object Positions and Object Rotations
The correspondence between vector id and board position is:
    //Tile x = a, z = b is at (b*bsize + a)
    //For real coordinates of (b*bsize + a) -> x = stepsize*(a-bOffset), z = stepsize*(b-bOffset)
    //0 - (bsize-1)         first z-Axis-row
    //bsize - (2bsize - 1)  second z-Axis-row and so forth*/

std::tuple < vector<ref_ptr<osg::Geode> >, int > createFloorTiles(Config config);
/*Creates and Returns all Floor Tile Objects and their total number
One Floor Tile object is created for each Object and 2 extra for Black and White*/

ref_ptr<osg::LightSource> makeLight(Config config);
/*Creates and returns the Light Source Object*/

ref_ptr<osgShadow::ShadowedScene> makeShadow(ref_ptr<osg::LightSource>, Config config);
/*Creates and returns the Shadowed Scene
Uses ShadowMap Shadow Type*/

std::tuple < vector<vector<ref_ptr<osg::Group> > >, vector<string>, vector<string>, int > loadObjects(Config config);
/*Returns all Objects, Their Names in Plural, Their Names in Singular and their total count
For each Object returns a 6-Unit vector for the colours*/

std::tuple < MatTransVec, MatTransVec, MatTransVec> createTranslationsNoShadow(ref_ptr<osg::Group> scene, vector<ref_ptr<osg::Geode> > floorTiles, Config config);
/*Shadowless Version for Compatibility
Creates and Returns all Floor Tile Positions, Object Positions and Object Rotations
The correspondence between vector id and board position is:
    //Tile x = a, z = b is at (b*bsize + a)
    //For real coordinates of (b*bsize + a) -> x = stepsize*(a-bOffset), z = stepsize*(b-bOffset)
    //0 - (bsize-1)         first z-Axis-row
    //bsize - (2bsize - 1)  second z-Axis-row and so forth*/


/*UpdateStep.h
Defines the two Classes that Handle Scene Updates and Anno/Ques-File Updates respectively*/

#pragma once
#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#include<ctime>
#endif
#include<osg/Node>
#include<osg/ref_ptr>
#include<osg/Group>
#include<osg/MatrixTransform>
#include<osg/Geode>
#include<osg/LightSource>
#include<vector>
#include<string>
#include<random>
#include<iostream>
#include<fstream>

#include"Utils.hpp"

using osg::ref_ptr;
using osg::Node;
using osg::Group;
using osg::MatrixTransform;

using std::vector;
using std::string;

using MatTransVec = vector<ref_ptr<MatrixTransform> >;


class SceneHandler {
	/*Controls the Scenegraph*/
public:
	SceneHandler(vector<vector<ref_ptr<osg::Group> > > objectsIn, vector<ref_ptr<osg::Geode> > floorTilesIn, MatTransVec translationsIn, MatTransVec rotationsIn, MatTransVec floorTileLocsIn, osg::Vec4 light, ref_ptr<osg::LightSource> lSrc, Config config);
	/*Loads all SceneGraph Objects into the Class*/
	void setRNG(std::mt19937* rngStreamIn, std::uniform_int_distribution<int>* objRandIn, std::uniform_int_distribution<int>* locRandIn, std::uniform_int_distribution<int>* colRandIn, std::uniform_real_distribution<double>* rotRandIn, std::uniform_int_distribution<int>* floorRandIn);
	/*Loads the RNG Pointers, separated out for readability purposes*/
	std::tuple<int, int, int> update(int objectID, int objectCount, int bannedObj);
	/*Updates the SceneGraph and returns 3 Different Objects present in the Constellation*/
	void setStaticPos();
	/*If the flag is set, creates and saves Static Positions*/
	void setChange();
	/*Loads Flow-Control-Flags from Config*/
private:
	Config config;

	vector<vector<ref_ptr<osg::Group> > >objects;	//All Object Nodes
	vector<ref_ptr<osg::Geode> > floorTiles;		//All Floor Tile Nodes
	vector<ref_ptr<MatrixTransform> > translations;	//All Object Positions
	vector<ref_ptr<MatrixTransform> > rotations;	//All Object Rotations
	vector<ref_ptr<MatrixTransform> > floorTileLocs;//All Floor Tile Positions

	bool colorChange;	//True means Objects change colour
	bool rotationChange;//True means Objects rotate
	bool positionChange;//True means Object Positions are not static
	bool noExtras;		//True means no extra Objects beyond the counted ones are loaded
	bool floorColors;	//True means the floor is coloured randomly
	bool firstPass;		//Debug Flag, If true, writes extended Debug info on first Update
	osg::Vec4 eyeLight;	//Position of the Light Source
	ref_ptr<osg::LightSource> lSrc;	//Light Source Pointer


	vector<vector<bool> > staticPositions;	//Static Positions as bool over the Board positions

	//RNGs
	std::mt19937* rngStream;
	std::uniform_int_distribution<int>* objRand;
	std::uniform_int_distribution<int>* locRand;
	std::uniform_int_distribution<int>* colRand;
	std::uniform_real_distribution<double>* rotRand;
	std::uniform_int_distribution<int>* floorRand;
};

class WriteHandler {
	/*Controls the OFStreams for the Annotation and Question File*/
public:
	WriteHandler(vector<string> objNamesSingular, vector<string> objNamesPlural, Config config);

	bool Init();	//Initializes the streams and calls the other two Init Methods
	bool InitQues();//Writes the Opening for the Question File
	bool InitAnno();//Writes the Opening for the Annotation File
	bool End();		//Calls the other two End Methods and closes both streams
	bool EndQues();	//Writes the Ending for the Question File
	bool EndAnno();	//Writes the Ending for the Annotation File

	bool QuesHM(int objID, int objAmount, string imageID);
	//Writes the Next "How Many" Question
	bool AnnoHM(int objID, int objAmount, string imageID);
	//Writes the Next "How Many" Annotation

	bool QuesAT(int objID, int objAmount, string imageID, bool truth);
	//Writes the Next "Are There" Question, Amount controls what Amount is asked for
	bool AnnoAT(int objID, int objAmount, string imageID, bool truth);
	//Wries the Next "Are there" Annotation, truth controls if the Answer is yes/no

	bool QuesIT(int objID, string imageID, bool truth);
	//Wries the Next "Is there" Question, truth controls if the Answer is yes/no
	bool AnnoIT(int objID, string imageID, bool truth);
	//Wries the Next "Is there" Annotation truth controls if the Answer is yes/no

private:
	vector<string> objNamesSingular;//All Object Names in the Singular
	vector<string> objNamesPlural;	//All Object Names in the Plural
	bool first;						//Required for proper JSON Formatting, if true, the opening comma is supressed
	int quesID;						//Question ID tracker
	std::ofstream quesOut;			//Outstream to the Question File
	std::ofstream annoOut;			//Outstream to the Annotation File

	Config config;					//Config

};


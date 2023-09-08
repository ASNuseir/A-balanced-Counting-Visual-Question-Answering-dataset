/*Init.cpp
* Defines Initialization Code for various OSG Objects
*/

#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif
#include"Init.h"



using std::string;

std::tuple < MatTransVec, MatTransVec, MatTransVec> createTranslations(ref_ptr<osgShadow::ShadowedScene> scene, vector<ref_ptr<osg::Geode> > floorTiles, Config config) {
	/*
	* Input:	Pointer to the Shadowed Scene, scene
	*			Vector of Pointers to each coloured floor tile, floorTiles
	*				Required only for uncolored Tiles, where they are statically assigned here
	*			Config, config
	* Output:	Vector of Pointers to each Object Position Node
	*			Vector of Pointers to each Object Rotation Node
	*			Vector of Pointers to each Tile Position Node
	*
	* Assembles All Positions of Rendered Entities in the SceneGraph
	*/
	
	int bsize = config.bsize;
	int stepsize = config.stepsize;

	MatTransVec translations;	//Holds the Pointers to each Object Position Node
	MatTransVec rotations;		//Holds the Pointers to each Object Rotation Node
	MatTransVec floorTileLocs;	//Holds the Pointers to each Floor Tile Position Node
	
	bool flipped = true;		//Alternates to Flip tile colours
	int bOffset = bsize / 2;	//Board is generated from the center outwards, this gives how many rows/columns there are beyond the central ones
	//Generate Chessboard Offsets
	cout << "Generating Chessboard\n";
	bool white = true;
	for (int i = 0; i < bsize; i++) {
		//Iterate over rows
		for (int j = 0; j < bsize; j++) {
			//Iterate over columns
			
			//Create Object Positions
			Matrix mat;
			ref_ptr<MatrixTransform> trans = new MatrixTransform;
				//Set Position, starting in a corner and then moving row by row
			mat.setTrans(-(bOffset * stepsize) + (j * stepsize), 0, -(bOffset * stepsize) + (i * stepsize));
			trans->setMatrix(mat);
				//Set Masks so attached Objects cast and receive shadows
			trans->setNodeMask(config.ReceivesShadowTraversalMask | config.CastsShadowTraversalMask);
				//Store in vector
			translations.push_back(trans);
			
			//Create Object Rotation
			Matrix rot;
			ref_ptr<MatrixTransform> rotator = new MatrixTransform;
				//Initialize without Rotation, this will be changed in each update step
			mat.makeRotate(0, osg::Vec3(0, 1, 0));
			rotator->setMatrix(mat);
				//Add Position as a child
			rotator->addChild(translations[i * bsize + j].get());
				//Set Masks so attached Objects cast and receive shadows
			rotator->setNodeMask(config.ReceivesShadowTraversalMask | config.CastsShadowTraversalMask);
				//Store in vector
			rotations.push_back(rotator);
				//Attach to shadowed scene
			scene->addChild(rotations[i * bsize + j].get());

			//Create Floor Position
			Matrix floormat;
			ref_ptr<MatrixTransform> floorTrans = new MatrixTransform;
			//These are identical to the Object Positions. Independence is for ease of handling
			floormat.setTrans(-(bOffset * stepsize) + (j * stepsize), /*-(double)stepsize / 2.0*/ 0, -(bOffset * stepsize) + (i * stepsize));
			floorTrans->setMatrix(floormat);
			//If Tile Colouring is disabled, attach checkerboard pattern to the positions
			if (config.noTileColors) {
				if (white) {
					floorTrans->addChild(floorTiles[0]);
					white = false;
				}
				else {
					floorTrans->addChild(floorTiles[1]);
					white = true;
				}
			}
			//Attach to Scene
			scene->addChild(floorTrans);
			floorTileLocs.push_back(floorTrans);
		}
	}

	return std::tuple<MatTransVec, MatTransVec, MatTransVec> (translations,rotations,floorTileLocs);
}

std::tuple < vector<ref_ptr<osg::Geode> >, int > createFloorTiles(Config config){
	/*Input:	Config config
	* Output:	Vector containing pointers to the individual coloured floor tiles
	*			Integer containing the number of floor tiles
	*/
	vector<ref_ptr<osg::Geode> > floorTiles;		//Stores Pointers to the coloured floor tiles

	//Generate basic tile geometry which is shared by all coloured Tiles
	ref_ptr<osg::Geometry> tile = new osg::Geometry;	//Basic Tile Object
	double corners = (double)config.stepsize / 2;		//Tile is centered on (0,0,0)
	ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;		//Holds the tile corners
	ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;//Holds the tile normal
	{
		//Define tile corners
		v->push_back(osg::Vec3(corners, 0.f, corners));	
		v->push_back(osg::Vec3(-corners, 0.f, corners));
		v->push_back(osg::Vec3(-corners, 0.f, -corners));
		v->push_back(osg::Vec3(corners, 0.f, -corners));
		//Set tile corners
		tile->setVertexArray(v.get());
		//Define normal
		normal->push_back(osg::Vec3(0, 1.f, 0));
		//Set Normal
		tile->setNormalArray(normal);
		tile->setNormalBinding(osg::Geometry::BIND_OVERALL);
		//Define what is being drawn
		tile->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
	}

	int tileCols = 0;							//Counts Number of Tiles
	cout << "Making Floor Tiles\n";
	if (!config.noTileColors) {
		string rgbVals;							//Holds the loaded value
		std::ifstream colsIn(config.tileColors);//Stream for file holding the RGB values
		//Define Floor Tiles
		while (!colsIn.eof()) {
			tileCols++;
			std::getline(colsIn, rgbVals);

			//Read RGB values from file
			string rStr, gStr, bStr;
			//Split by spaces
			rStr = rgbVals.substr(0, rgbVals.find(" "));
			rgbVals = rgbVals.substr(rgbVals.find(" ") + 1, string::npos);
			gStr = rgbVals.substr(0, rgbVals.find(" "));
			bStr = rgbVals.substr(rgbVals.find(" ") + 1, string::npos);
			//Convert to int
			int rInt, gInt, bInt;
			rInt = std::stoi(rStr);
			gInt = std::stoi(gStr);
			bInt = std::stoi(bStr);
			//Convert from 0~255 to 0~1
			float r = (float)rInt;
			float g = (float)gInt;
			float b = (float)bInt;
			r = r / 255;
			g = g / 255;
			b = b / 255;

			//Create New Floor Tile
			ref_ptr<osg::Geode> floorTile = new osg::Geode;
			floorTile->setNodeMask(~config.CastsShadowTraversalMask);

			//Create New Material
			ref_ptr<osg::Material> color = new osg::Material;
			color->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(r, g, b, 1));
			color->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(r, g, b, 1));
			color->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
			color->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
			color->setShininess(osg::Material::Face::FRONT_AND_BACK, 50);
			ref_ptr<osg::StateSet> stateColor = new osg::StateSet;
			stateColor->setAttribute(color, osg::StateAttribute::OVERRIDE);

			//Attach Material to floor tile
			floorTile->setStateSet(stateColor);
			//Attach tile to floor tile
			floorTile->addDrawable(tile);
			//Store in Vector
			floorTiles.push_back(floorTile);

		}
	}
	else {
		//Create one White and one Orange Tile by the same method as above

		ref_ptr<osg::Geode> floorTileWhite = new osg::Geode;
		floorTileWhite->setNodeMask(~config.CastsShadowTraversalMask);
		ref_ptr<osg::Material> colorWhite = new osg::Material;
		colorWhite->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
		colorWhite->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
		colorWhite->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
		colorWhite->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
		colorWhite->setShininess(osg::Material::Face::FRONT_AND_BACK, 50);
		ref_ptr<osg::StateSet> stateColorWhite = new osg::StateSet;
		stateColorWhite->setAttribute(colorWhite, osg::StateAttribute::OVERRIDE);
		floorTileWhite->setStateSet(stateColorWhite);


		floorTileWhite->addDrawable(tile);
		floorTiles.push_back(floorTileWhite);

		ref_ptr<osg::Geode> floorTileOrange = new osg::Geode;
		floorTileOrange->setNodeMask(~config.CastsShadowTraversalMask);
		ref_ptr<osg::Material> colorOrange = new osg::Material;
		colorOrange->setAmbient(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 0.5, 0, 1));
		colorOrange->setDiffuse(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(1, 0.5, 0, 1));
		colorOrange->setSpecular(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
		colorOrange->setEmission(osg::Material::Face::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
		colorOrange->setShininess(osg::Material::Face::FRONT_AND_BACK, 50);
		ref_ptr<osg::StateSet> stateColorOrange = new osg::StateSet;
		stateColorOrange->setAttribute(colorOrange, osg::StateAttribute::OVERRIDE);
		floorTileOrange->setStateSet(stateColorOrange);


		floorTileOrange->addDrawable(tile);
		floorTiles.push_back(floorTileOrange);
	}

	return std::tuple<vector<ref_ptr<osg::Geode> >, int>(floorTiles, tileCols);
}

ref_ptr<osg::LightSource> makeLight(Config config) {
	/*Input:	Config config
	* Output:	Pointer to a Light Source
	*/
	ref_ptr<osg::LightSource> lSrc = new osg::LightSource;
	ref_ptr<osg::Light> light = new osg::Light;
	osg::Vec4 lpos = osg::Vec4(0, 30, 0, 1);	//Position gets overriden later
	light->setLightNum(0);						//Required for OpenGL/Shadows
	//Set Ambient Component
	light->setAmbient(osg::Vec4(config.aBright, config.aBright, config.aBright, 1));
	//Set Other Components
	light->setDiffuse(osg::Vec4(1, 1, 1, 1));
	light->setSpecular(osg::Vec4(0, 0, 0, 1));
	light->setPosition(lpos);
	light->setDirection(osg::Vec3(0, -1, 0));	//Gets overridden later
	//Disable all attenuation (falloff with distance)
	light->setConstantAttenuation(1);
	light->setLinearAttenuation(0);
	light->setQuadraticAttenuation(0);
	//Attach to Light source
	lSrc->setLight(light);
	return lSrc;
}

ref_ptr<osgShadow::ShadowedScene> makeShadow(ref_ptr<osg::LightSource> lSrc, Config config) {
	/*Input:		Pointer to the initialized Light Source	lSrc
	*				Config config
	* Output:		Pointer to the initialized ShadowedScene Node, not attached to main scene node yet
	*/
	osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;//Shadowed Scene Pointer
	osg::ref_ptr<osgShadow::ShadowMap> sm = new osgShadow::ShadowMap;					//Shadow Type Shadow Map
	sm->setLight(lSrc);												//Light which the shadow is calculated from
	sm->setTextureSize(osg::Vec2s(4096, 4096));						//Shadow resolution (recommended by example)
	sm->setAmbientBias(osg::Vec2(config.aBias, 1 - config.aBias));	//Shadow Intensity, higher first value is stronger shadow
	shadowedScene->setShadowTechnique(sm.get());										//Attach to shadowed scene
	shadowedScene->setReceivesShadowTraversalMask(config.ReceivesShadowTraversalMask);	//Set Shadow Masks
	shadowedScene->setCastsShadowTraversalMask(config.CastsShadowTraversalMask);
	shadowedScene->addChild(lSrc);														//Attach Light Source
	return shadowedScene;
}

std::tuple < vector<vector<ref_ptr<osg::Group> > >, vector<string>, vector<string>, int > loadObjects(Config config) {
	/*Input:	Config config
	* Output:	Vector of vectors of Pointers to the individual coloured in objects
	*/
	std::cout << "Loading Objects\n";
	vector<vector<ref_ptr<osg::Group> > > objects;	//Holds Pointers to the individual Colour Groups that the Object Nodes are attached to
	vector<string> objNames;						//Holds the object names in the plural
	vector<string> objSingular;						//Holds the object names in the singular
	//Open File
	string content;
	std::ifstream fileIn(config.target);
	std::getline(fileIn, content);
	int objCount = std::stoi(content);	//Read the number of objects

	//Create 5 Materials, then produce one state-set for each material and each object
	//Instead of creating 5*Object Objects
	vector<ref_ptr<osg::StateSet> > chroma;
	std::cout << "Making Colours\n";
	chroma = makeColors();

	//Load Object
	//Clone Object in different colours
	std::cout << "Loading and colouring objects\n";
	for (int i = 0; i < objCount; i++) {
		//Get Object Position
		std::getline(fileIn, content);
		//Get Name Plural
		string name;
		std::getline(fileIn, name);
		//Get Name Singular
		string singular;
		std::getline(fileIn, singular);
		//Store Names in vectors
		objNames.push_back(name);
		objSingular.push_back(singular);
		//Read Node from File
		ref_ptr<Node> actNode = osgDB::readNodeFile(content);
		//Set Shadow Mask
		actNode->setNodeMask(config.ReceivesShadowTraversalMask | config.CastsShadowTraversalMask);
		//Vector will hold the 7 Color Groups
		vector<ref_ptr<osg::Group> > actNodeVec;
		//Create No Colour Group
		ref_ptr<osg::Group> noCol = new osg::Group;
		noCol->setNodeMask(config.ReceivesShadowTraversalMask | config.CastsShadowTraversalMask);
		//Add Object to No Colour Group
		noCol->addChild(actNode);
		actNodeVec.push_back(noCol);

		for (int j = 0; j < 6; j++) {
			//Create Colour Group
			ref_ptr<osg::Group> colGroup = new osg::Group;
			colGroup->setNodeMask(config.ReceivesShadowTraversalMask | config.CastsShadowTraversalMask);
			//Add Child
			colGroup->addChild(actNode);
			//Set Colour
			colGroup->setStateSet(chroma[j].get());
			actNodeVec.push_back(colGroup);
		}
		//Store in Output Vector
		objects.push_back(actNodeVec);
	}

	return std::tuple< vector<vector<ref_ptr<osg::Group> > >, vector<string>, vector<string>, int>(objects, objNames, objSingular, objCount);
}

std::tuple < MatTransVec, MatTransVec, MatTransVec> createTranslationsNoShadow(ref_ptr<osg::Group> scene, vector<ref_ptr<osg::Geode> > floorTiles, Config config) {
	/*
	* Input:	Pointer to the Shadowed Scene, scene
	*			Vector of Pointers to each coloured floor tile, floorTiles
	*				Required only for uncolored Tiles, where they are statically assigned here
	*			Config, config
	* Output:	Vector of Pointers to each Object Position Node
	*			Vector of Pointers to each Object Rotation Node
	*			Vector of Pointers to each Tile Position Node
	*
	* Assembles All Positions of Rendered Entities in the SceneGraph
	*/
	
	int bsize = config.bsize;
	int stepsize = config.stepsize;

	MatTransVec translations;	//Holds the Pointers to each Object Position Node
	MatTransVec rotations;		//Holds the Pointers to each Object Rotation Node
	MatTransVec floorTileLocs;	//Holds the Pointers to each Floor Tile Position Node
	
	bool flipped = true;		//Alternates to Flip tile colours
	int bOffset = bsize / 2;	//Board is generated from the center outwards, this gives how many rows/columns there are beyond the central ones
	//Generate Chessboard Offsets
	cout << "Generating Chessboard\n";
	bool white = true;
	for (int i = 0; i < bsize; i++) {
		//Iterate over rows
		for (int j = 0; j < bsize; j++) {
			//Iterate over columns
			
			//Create Object Positions
			Matrix mat;
			ref_ptr<MatrixTransform> trans = new MatrixTransform;
				//Set Position, starting in a corner and then moving row by row
			mat.setTrans(-(bOffset * stepsize) + (j * stepsize), 0, -(bOffset * stepsize) + (i * stepsize));
			trans->setMatrix(mat);
				//Set Masks so attached Objects cast and receive shadows
			trans->setNodeMask(config.ReceivesShadowTraversalMask | config.CastsShadowTraversalMask);
				//Store in vector
			translations.push_back(trans);
			
			//Create Object Rotation
			Matrix rot;
			ref_ptr<MatrixTransform> rotator = new MatrixTransform;
				//Initialize without Rotation, this will be changed in each update step
			mat.makeRotate(0, osg::Vec3(0, 1, 0));
			rotator->setMatrix(mat);
				//Add Position as a child
			rotator->addChild(translations[i * bsize + j].get());
				//Set Masks so attached Objects cast and receive shadows
			rotator->setNodeMask(config.ReceivesShadowTraversalMask | config.CastsShadowTraversalMask);
				//Store in vector
			rotations.push_back(rotator);
				//Attach to shadowed scene
			scene->addChild(rotations[i * bsize + j].get());

			//Create Floor Position
			Matrix floormat;
			ref_ptr<MatrixTransform> floorTrans = new MatrixTransform;
			//These are identical to the Object Positions. Independence is for ease of handling
			floormat.setTrans(-(bOffset * stepsize) + (j * stepsize), /*-(double)stepsize / 2.0*/ 0, -(bOffset * stepsize) + (i * stepsize));
			floorTrans->setMatrix(floormat);
			//If Tile Colouring is disabled, attach checkerboard pattern to the positions
			if (config.noTileColors) {
				if (white) {
					floorTrans->addChild(floorTiles[0]);
					white = false;
				}
				else {
					floorTrans->addChild(floorTiles[1]);
					white = true;
				}
			}
			//Attach to Scene
			scene->addChild(floorTrans);
			floorTileLocs.push_back(floorTrans);
		}
	}

	return std::tuple<MatTransVec, MatTransVec, MatTransVec> (translations,rotations,floorTileLocs);
}
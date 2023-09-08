/*Utils.hpp
Contains Utility functions*/

#pragma once
#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif

#include<osg/Material>
#include<osg/StateSet>
#include<osg/ref_ptr>
#include<osg/NodeVisitor>
#include<osg/Geode>
#include<osg/Node>
#include<osgDB/WriteFile>

#include<vector>
#include<string>
#include<iostream>
#include<random>
#include<fstream>
#include<cmath>


using std::vector;
using std::string;
using osg::ref_ptr;
using std::cout;

vector<ref_ptr<osg::StateSet> > makeColors();
/*Creates and returns the 5 Colors used for this Object as StateSets*/
string leftpad(int input, int padTo);
string pad4(int input);//Leftpad to 4
string pad3(int input);//Leftpad to 3
string pad2(int input);//Leftpad to 2

class WindowCaptureCallback : public osg::Camera::DrawCallback
/*Captures the given Buffer under the given Filename
Code taken entirely from OSG-Example autocapture.cpp*/
{
public:
    WindowCaptureCallback(GLenum readBuffer, string fileName) :
        _readBuffer(readBuffer),
        _fileName(fileName)
    {
        _image = new osg::Image;
    }

    virtual void operator () (osg::RenderInfo& renderInfo) const
    {
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
        glReadBuffer(_readBuffer);
#else
        osg::notify(osg::NOTICE) << "Error: GLES unable to do glReadBuffer" << std::endl;
#endif

        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
        if (gc->getTraits())
        {
            GLenum pixelFormat;

            if (gc->getTraits()->alpha)
                pixelFormat = GL_RGBA;
            else
                pixelFormat = GL_RGB;

#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
            if (pixelFormat == GL_RGB)
            {
                GLint value = 0;
#ifndef GL_IMPLEMENTATION_COLOR_READ_FORMAT
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#endif
                glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &value);
                if (value != GL_RGB ||
                    value != GL_UNSIGNED_BYTE)
                {
                    pixelFormat = GL_RGBA;//always supported
                }
            }
#endif
            int width = gc->getTraits()->width;
            int height = gc->getTraits()->height;

            std::cout << "Capture: size=" << width << "x" << height << ", format=" << (pixelFormat == GL_RGBA ? "GL_RGBA" : "GL_RGB") << std::endl;

            _image->readPixels(0, 0, width, height, pixelFormat, GL_UNSIGNED_BYTE);
        }

        if (!_fileName.empty())
        {
            std::cout << "Writing to: " << _fileName << std::endl;
            osgDB::writeImageFile(*_image, _fileName);
        }
    }
protected:
    string                      _fileName;
    GLenum                      _readBuffer;
    osg::ref_ptr<osg::Image>    _image;
    mutable OpenThreads::Mutex  _mutex;
};

class Config {
    /*Holds all CL-Parameters for use throughout the Programm*/
public:
    Config();
    bool verify();
    /*Verifies all required Parameters were given*/
    bool printFile();
    /*Prints a Config File*/

    string target;          //Position of file that contains all Object File Location
    string tileColors;      //File that contains the RGB values for Characteristic Colors/Floor Tile Colors
    bool train;             //If true, train mode
    bool val;               //If true, validation mode
    bool staticPositions;   //If true, Object Positions are static
    bool staticRotations;   //If true, Objects don't rotate
    bool noColor;           //If true, Objects stay in their original colour
    bool noExtras;          //If true, does not load extra objects
    bool staticCam;         //If true, disables camera rotation
    bool noTileColors;      //If true, disables tile coloring
    bool oldSplit;          //If true, switches back to old split

    int seed;               //Main seed for everything
    int staticSeed;         //Seed for static Object positions

    int constellations;     //Number of constellations per Object+Count Pair
    int bsize;              //Edge Length of the board in tiles
    int stepsize;           //Edge Length of a tile

    string identifier;      //Identifier code
    string imagePath;       //Path where images are saved

    double aBias;           //Shadow intensity
    double aBright;         //Ambient Light Brightness

    bool extDebug;          //Turns on Extended Debug Mode
    bool noShader;          //Disables Shader as Emergency Measure

    int ReceivesShadowTraversalMask = 0x1;  //Flag for Shadow purposes
    int CastsShadowTraversalMask = 0x2;     //Flag for Shadow purposes
};
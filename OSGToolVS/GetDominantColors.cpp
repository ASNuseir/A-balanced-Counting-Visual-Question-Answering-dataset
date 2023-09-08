/*GetDominantColors.cpp
* Requires OpenCV, guaranteed to work 4.3.0
* For all given Objects does the following:
* - Load Object, Screenshot Object
* - Load Screenshot
* - Take Colour histogram
* - Find maximum of histogram
* - Save to file
*/

#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif

#include<osg/Node>
#include<osg/Group>
#include<osg/ref_ptr>
#include<osgViewer/Viewer>
#include<osgDB/WriteFile>
#include<osgDB/ReadFile>
#include<opencv2/imgproc.hpp>
#include<opencv2/imgcodecs.hpp>

#include<iostream>
#include<string>
#include"CLI11.hpp"

using osg::ref_ptr;

using std::cout;
using std::string;


class WindowCaptureCallback : public osg::Camera::DrawCallback
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

            //std::cout << "Capture: size=" << width << "x" << height << ", format=" << (pixelFormat == GL_RGBA ? "GL_RGBA" : "GL_RGB") << std::endl;

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

int main(int argc, char* argv[]) {
    string target = "";

    CLI::App app{ "Tool to generate datasets to test counting in VisualQA." };
    app.add_option("-t,--target", target, "File containing the objects to load.");


    CLI11_PARSE(app, argc, argv);

    if (target == "Unassigned") {
        cout << "Please specify models to load." << std::endl;
        return -1;
    }
    else {
        cout << "Target is " << target << "." << std::endl;
    }

    //Before Loop, get background color

    osgViewer::Viewer viewer;
    ref_ptr<osg::Group> scene = new osg::Group;
    scene->setDataVariance(osg::Object::DataVariance::DYNAMIC);

    //Create Pixelbuffer
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

    //Get Background colour and convert to OpenCV Represenation 0~255 from OSG 0~1
    osg::Vec4 bgColor = viewer.getCamera()->getClearColor();
    std::cout << 255*bgColor.b() << std::endl;
    std::cout << 255*bgColor.r() << std::endl;
    std::cout << 255*bgColor.g() << std::endl;
    cv::Scalar bgColorCV(255*bgColor.b(), 255*bgColor.g(), 255 * bgColor.r());
    cv::Mat pureBG(100, 100, CV_8UC3, bgColorCV);   

    cv::imwrite("DebugBG.png", pureBG); //Write Debug square


    //Create Lightsource
    osg::Vec4f lightEye(40, 80, 80, 1);
    ref_ptr<osg::LightSource> lSrc = new osg::LightSource;
    ref_ptr<osg::Light> light = new osg::Light;
    {;
        light->setLightNum(0);
        light->setAmbient(osg::Vec4(1, 1, 1, 1));
        light->setDiffuse(osg::Vec4(1, 1, 1, 1));
        light->setSpecular(osg::Vec4(0, 0, 0, 1));
        light->setPosition(lightEye);
        light->setDirection(osg::Vec3(-0.5, -1, -1));
        light->setConstantAttenuation(1);
        light->setLinearAttenuation(0);
        light->setQuadraticAttenuation(0);
        lSrc->setLight(light);
    }
    scene->addChild(lSrc);
    /*cv::Mat loadedIm = cv::imread("DebugBG.png", cv::IMREAD_COLOR);
    int channels[] = { 0,1,2};
    cv::Mat histogram;
    int histSize[] = { 256, 256, 256};
    float bRange[] = { 0.0, 256.0 };
    float gRange[] = { 0.0, 256.0 };
    float rRange[] = { 0.0, 256.0 };
    const float* ranges[] = { bRange, gRange, rRange};
    cv::calcHist(&loadedIm, 1, channels, cv::Mat(), histogram, 3, histSize, ranges, true, false);
    for (int i = 0; i < 256; i++) {
        bool printFrame = false;
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 256; k++) {
                float val = histogram.at<float>(i, j, k);
                if (val != 0) {
                    std::cout << i << " " << j << " " << k << ":";
                    std::cout << val << std::endl;
                }
            }
        }
    }*/

    
    //Load Object
    string content;
    string junk;
    std::ifstream fileIn(target);   //In stream for the Object Location File
    std::getline(fileIn, content);
    int objCount = std::stoi(content);
    viewer.setSceneData(scene);
    viewer.realize();

    //Initialize Camera to look at Object
    osg::Vec3f baseEye(0, 80, 80);
    viewer.getCamera()->setViewMatrixAsLookAt(baseEye, osg::Vec3f(0, 0, 0), osg::Vec3f(0, 1, 0));

    std::ofstream colorsOut;        //Output Stream for the Color File
    colorsOut.open("ColorsRGB.txt");

    for (int i = 0; i < objCount; i++) {
        std::getline(fileIn, content);  //Get Object position
        std::getline(fileIn, junk);     //discard name
        std::getline(fileIn, junk);     //discard name
        scene->removeChild(1);          //Delete Debug Child just in case
        ref_ptr<osg::Node> object = osgDB::readNodeFile(content);   //Load Object
        scene->addChild(object);

        //Take Screenshot
        GLenum buffer = viewer.getCamera()->getGraphicsContext()->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
        string filename = "./FloorTileColors/" + std::to_string(i) + ".png";
        viewer.frame();
        viewer.getCamera()->setFinalDrawCallback(new WindowCaptureCallback(buffer, filename));
        viewer.frame();
        viewer.getCamera()->setFinalDrawCallback(NULL);

        //Screenshot Object

        //Load Screenshot
        std::cout << "Loaded Image: " << filename << std::endl;
        cv::Mat loadedIm = cv::imread(filename, cv::IMREAD_COLOR);
        cv::imwrite("./FloorTileColors/" + std::to_string(i) + "debug.png", loadedIm); //Write Image as loaded by OpenCV

        //Define All necessary values for a 26 Bucket per Channel histogram over 3 Channels
        cv::MatND histogram;
        int channels[] = { 0,1,2 };
        int histSize[] = { 26, 26, 26 };
        float bRange[] = { 0, 256 };
        float gRange[] = { 0, 256 };
        float rRange[] = { 0, 256 };
        const float* ranges[] = { bRange, gRange, rRange };
        //Calculate the Histogram
        cv::calcHist(&loadedIm, 1, channels, cv::Mat(), histogram, 3, histSize, ranges, true, false);
        
        //Zero the Bucket of the Background Colour
        histogram.at<float>((int)(bgColorCV[0]/10), (int)(bgColorCV[1]/10), (int)(bgColorCV[2]/10)) = 0;
        //Zero the White and Black Bucket
        histogram.at<float>(0, 0, 0) = 0;
        histogram.at<float>(25, 25, 25) = 0;
        //Output Check that Histogram at BG Color is 0, assume the other two are also 0 by virtue of working the same
        std::cout << "Checksum: " << histogram.at<float>((int)(bgColorCV[0] / 10), (int)(bgColorCV[1] / 10), (int)(bgColorCV[2] / 10)) << std::endl;
        
        //Prepare necessary values to find the Maximum Bucket
        double minVal;
        double maxVal;
        cv::Point minPoint;
        cv::Point maxPoint;
        cv::minMaxLoc(histogram.reshape(1,1), &minVal, &maxVal, &minPoint, &maxPoint);
        //The Histogram needs to be 2D for this to work, hence we reshape it into one single column
        //This effectively encodes the RGB Value into the position in the column a decimal rep. of a Base 26 number
        std::cout << "Highest Value: " << maxVal << std::endl;
        std::cout << "x: " << static_cast<int>(maxPoint.x) << " y: " << static_cast<int>(maxPoint.y) << std::endl;

        //Convert from Decimal into Base 26 for RGB Values
        int toBGR = maxPoint.x;
        int r = (toBGR % 26)*10;
        toBGR = toBGR / 26;
        int g = (toBGR % 26)*10;
        toBGR = toBGR / 26;
        int b = toBGR*10;
        //Print to Screen
        std::cout << "BGR: (" << b << "," << g << "," << r << ")" << std::endl;
        //Print Demo Square of the Color
        cv::Mat debugColor(100, 100, CV_8UC3, cv::Scalar(b, g, r));
        cv::imwrite("./FloorTileColors/" + std::to_string(i) + "rawColor.png", debugColor);
        if (i != 0) {
            colorsOut << std::endl;
        }
        //Print to File
        colorsOut << r << " " << g << " " << b;

    }

    //Add White and Black as colours
    colorsOut << std::endl << "1 1 1" << std::endl << "255 255 255";
    
    colorsOut.close();


}
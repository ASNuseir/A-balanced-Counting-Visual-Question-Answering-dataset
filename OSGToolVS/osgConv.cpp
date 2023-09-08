/*osgConv.cpp
* Normalizes all given files to the same size, orientation and position
*/

#ifdef _WIN32
#define _USE_MATH_DEFINES // for C++
#include<windows.h>
#endif
#include<osg/Node>
#include<osg/Group>
#include<osg/Material>
#include<osg/StateSet>
#include<osg/MatrixTransform>
#include<osgDB/ReadFile>
#include<osgDB/WriteFile>
#include<osgUtil/SmoothingVisitor>

#include<string>
#include<iostream>
#include<fstream>
#include"CLI11.hpp"
#include"Utils.hpp"

using std::cout;
using std::string;
using std::ifstream;
using std::endl;

class BoundingBoxVisitor : public osg::NodeVisitor {
public:
    BoundingBoxVisitor(osg::BoundingBox *input) {
        _BB = input;
        _BB->init();
        setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    }
    void apply(osg::Geode& Geode) {
        _BB->expandBy(Geode.getBoundingBox());
        traverse(Geode);
    }
    osg::BoundingBox *_BB;
};

int main(int argc, char *argv[]){

    string target = "";
    int size = 20;
    bool debug = false;
    CLI::App app{ "Tool to generate datasets to test counting in VisualQA." };
    app.add_flag("-d,--debug", debug, "Enables debug mode");
    app.add_option("-t,--target", target, "File containing the objects to load.");
    app.add_option("-s,--size", size, "Radius to which objects are rescaled");
    CLI11_PARSE(app, argc, argv);

    if (target == "") {
        cout << "Please specify models to load.\n";
    }
    else {
        cout << "Target is " << target << "\n";
    }
    cout << "Radius is " << size << "\n";

    

    ifstream input(target);
    string actTarget;
    std::vector<osg::ref_ptr<osg::StateSet> > chroma = makeColors();

    while(!input.eof()){
        //Get File
        osg::ref_ptr<osg::Group> scene = new osg::Group;
        std::getline(input, actTarget);
        cout << "Active Target: " << actTarget << "\n";
        //Read Object from file
        osg::ref_ptr<osg::Node>targNode = osgDB::readNodeFile(actTarget);
        osgUtil::SmoothingVisitor smooth;
        targNode->accept(smooth);
        cout << "File read\n";
        //Debug Nodes
        osg::ref_ptr<osg::Group>targ = new osg::Group;
        osg::ref_ptr<osg::Group>targDebug1 = new osg::Group;
        osg::ref_ptr<osg::Group>targDebug2 = new osg::Group;
        osg::ref_ptr<osg::Group>targDebug3 = new osg::Group;
        if (debug) {
            targ->setStateSet(chroma[3].get());       //Purple
            targDebug1->setStateSet(chroma[0].get()); //Red
            targDebug2->setStateSet(chroma[2].get()); //Blue
            targDebug3->setStateSet(chroma[1].get()); //Green
            targDebug1->addChild(targNode.get());
            targDebug2->addChild(targNode.get());
            targDebug3->addChild(targNode.get());
        }
        targ->addChild(targNode.get());


        //Scale to uniform size given by -s
        osg::BoundingSphere sphere = targ->getBound();  //Get Bounding Sphere
        int radius = sphere.radius();                   //Get Radius
        float scale = (float)size / (float)radius;      //Get Norm Factor
        osg::Matrix transcale;
        transcale.makeScale(scale, scale, scale);       //Scale
        osg::ref_ptr<osg::MatrixTransform> tscale = new osg::MatrixTransform;
        tscale->setMatrix(transcale);
        tscale->addChild(targ.get());

        //Save to Debug
        osg::ref_ptr<osg::Group> debugScale = new osg::Group;
        osg::ref_ptr<osg::MatrixTransform> tscaleDebug1 = new osg::MatrixTransform;
        osg::ref_ptr<osg::MatrixTransform> tscaleDebug2 = new osg::MatrixTransform;
        osg::ref_ptr<osg::MatrixTransform> tscaleDebug3 = new osg::MatrixTransform;
        if (debug) {
            tscaleDebug1->setMatrix(transcale);
            tscaleDebug1->addChild(targDebug1.get());
            tscaleDebug2->setMatrix(transcale);
            tscaleDebug2->addChild(targDebug2.get());
            tscaleDebug3->setMatrix(transcale);
            tscaleDebug3->addChild(targDebug3.get());
            debugScale->addChild(tscaleDebug1.get());
        }
        cout << "Scale: " << scale << "\n";
        cout << "Node scaled\n";

        //Center properly
        sphere = tscale->getBound();        //Get Sphere
        osg::Vec3 center = sphere.center(); //Get Sphere Center
        osg::Matrix transcenter; 
        transcenter.makeTranslate(-center); //Move Center to Origin
        osg::ref_ptr<osg::MatrixTransform> tcenter = new osg::MatrixTransform;
        tcenter->setMatrix(transcenter);
        tcenter->addChild(tscale.get());
        cout << "Displacement: " << center.x() << ", " << center.y() << ", " << center.z() << "\n";
        cout << "Node Centered\n";

        //Debug
        osg::ref_ptr<osg::MatrixTransform> tcenterDebug2 = new osg::MatrixTransform;
        osg::ref_ptr<osg::MatrixTransform> tcenterDebug3 = new osg::MatrixTransform;
        osg::ref_ptr<osg::Group> debugCenter = new osg::Group;
        if (debug) {
            tcenterDebug2->setMatrix(transcenter);
            tcenterDebug2->addChild(tscaleDebug2.get());
            tcenterDebug3->setMatrix(transcenter);
            tcenterDebug3->addChild(tscaleDebug3.get());
            debugCenter->addChild(tcenterDebug2.get());
        }

        //Place on Floor
        //Get Bounding Box using Visitor
        osg::BoundingBox *BBox = new osg::BoundingBox;
        BoundingBoxVisitor bbVis(BBox);
        tcenter->accept(bbVis);
        cout << "X Range:" << BBox->xMin() << " ~ " << BBox->xMax() << "\n";
        cout << "Y Range:" << BBox->yMin() << " ~ " << BBox->yMax() << "\n";
        cout << "Z Range:" << BBox->zMin() << " ~ " << BBox->zMax() << "\n";

        //Place on Floor by 0ing z Component of Bounding Box
        //To do this, it must undergo the same steps the object already has
        osg::Vec3 toFloor(0, 0, -BBox->zMin());
        cout << "To Floor:\n";
        cout << "\tRaw: (" << toFloor.x() << ", " << toFloor.y() << ", " << toFloor.z() << ")" << endl;
        toFloor = transcale * toFloor;
        cout << "\tAfter Scaled: (" << toFloor.x() << ", " << toFloor.y() << ", " << toFloor.z() << ")" << endl;
        toFloor = toFloor + osg::Vec3(0,0,center.z());
        cout << "\tAfter Centered: (" << toFloor.x() << ", " << toFloor.y() << ", " << toFloor.z() << ")" << endl;
        osg::Matrix transFloor;
        transFloor.makeTranslate(toFloor);
        osg::ref_ptr<osg::MatrixTransform> tFloor = new osg::MatrixTransform;
        tFloor->setMatrix(transFloor);
        tFloor->addChild(tcenter.get());
        cout << "Snapped to Floor\n";

        //Debug
        osg::ref_ptr<osg::Geode> boxFloor = new osg::Geode;
        if (debug) {
            ref_ptr<osg::Geometry> tile = new osg::Geometry;
            ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
            v->push_back(osg::Vec3(scale * BBox->xMin(), scale * BBox->yMin(), scale * BBox->zMin()));
            v->push_back(osg::Vec3(scale * BBox->xMax(), scale * BBox->yMin(), scale * BBox->zMin()));
            v->push_back(osg::Vec3(scale * BBox->xMin(), scale * BBox->yMax(), scale * BBox->zMin()));
            v->push_back(osg::Vec3(scale * BBox->xMax(), scale * BBox->yMax(), scale * BBox->zMin()));
            tile->setVertexArray(v.get());

            ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
            normal->push_back(osg::Vec3(0, 1.f, 0));
            tile->setNormalArray(normal);
            tile->setNormalBinding(osg::Geometry::BIND_OVERALL);

            tile->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
            boxFloor->addDrawable(tile);
        }
        osg::ref_ptr<osg::Group> debugFloor = new osg::Group;
        osg::ref_ptr<osg::MatrixTransform> tFloorDebug3 = new osg::MatrixTransform;
        if (debug) {
            tFloorDebug3->setMatrix(transFloor);
            tFloorDebug3->addChild(tcenterDebug3.get());
            debugFloor->addChild(tFloorDebug3.get());
        }


        //Rotate y-up 90 degrees
        osg::Matrix rotate;
        rotate.makeRotate(-M_PI * 0.5, osg::Vec3(1, 0, 0));
        osg::ref_ptr<osg::MatrixTransform> trotate = new osg::MatrixTransform;
        trotate->setMatrix(rotate);
        trotate->addChild(tFloor.get());
        cout << "Node Rotated\n";
        scene->addChild(trotate.get());

        //Get Write Location
        string file = "";
        string root = "";
        {
            std::size_t pos;
            pos = actTarget.rfind('\\');
            if (pos != string::npos) {
                root = actTarget.substr(0, pos+1);
                file = actTarget.substr(pos + 1, string::npos);
            }
            else {
                pos = actTarget.rfind('/');
                if (pos != string::npos) {
                    root = actTarget.substr(0, pos+1);
                    file = actTarget.substr(pos + 1, string::npos);
                }
            }
            pos = file.rfind('.');
            if (pos != string::npos) {
                std::cout << file << std::endl;
                file = file.substr(0, pos);
                std::cout << file << std::endl;
            }
            else {
                std::cout << "Resolve failure\n";
            }
        }
        //Write to same place File came from
        string outMain = root + file + ".osg";
        if (debug) {
            string outDebug = root + "Debug/" + file;
            osgDB::writeNodeFile(*tFloorDebug3.get(), outDebug + "3.osg");
            osgDB::writeNodeFile(*tcenterDebug2.get(), outDebug + "2.osg");
            osgDB::writeNodeFile(*tscaleDebug1.get(), outDebug + "1.osg");
            osgDB::writeNodeFile(*boxFloor.get(), outDebug + "F.osg");
        }

        //Write
        osgDB::writeNodeFile(*scene.get(), outMain);
        cout << "Write location: " << outMain << "\n";
    }
    
}
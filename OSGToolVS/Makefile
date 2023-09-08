ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    detected_OS := Windows
else
    detected_OS := $(shell uname)  # same as "uname -s"
endif

libOSG = -losg -losgDB -losgViewer -losgGA -losgShadow -lOpenThreads
libOpenCV = -lopencv_core430 -lopencv_imgcodecs430 -lopencv_imgproc430
ifeq ($(detected_OS), Windows)
	libOS = -lopengl32
else
	libOS = -framework OpenGL -framework Cocoa
endif

ifeq ($(detected_OS), Windows)
	libPathOSG = -L"C:\Users\morit\Desktop\Bachelor\OpenSceneGraph-3.6.5\lib"
	incPaths = -I"C:\Users\morit\Desktop\Bachelor\OpenSceneGraph-3.6.5\include"
else
	libPathOSG = -L"/opt/local/lib"
	libPathCV = -L"/opt/local/lib"
	incPaths = -I"/opt/local/include"
endif
CXX = -std=c++14

MainShell.exe: MainShell.o UpdateStep.o Utils.o Init.o
	g++ $(CXX) -o MainShell.exe UpdateStep.o Utils.o Init.o Mainshell.o $(incPaths) $(libPathOSG) $(libOSG) $(libOS)

MainShell.o: MainShell.cpp
	g++ $(CXX) -c MainShell.cpp $(incPaths)

UpdateStep.o: UpdateStep.cpp
	g++ $(CXX) -c UpdateStep.cpp $(incPaths)

Utils.o: Utils.cpp
	g++ $(CXX) -c Utils.cpp $(incPaths)

Init.o: Init.cpp
	g++ $(CXX) -c Init.cpp $(incPaths)

Testing.exe: Testing.o
	g++ $(CXX) -o Testing.exe Testing.o $(incPaths) $(libPathOSG) $(libOSG)

Testing.o: Testing.cpp
	g++ $(CXX) -c Testing.cpp $(incPaths)

Conv.exe: osgConv.o Utils.o
	g++ $(CXX) -o Conv.exe osgConv.o $(incPaths) $(libPathOSG) $(libOSG)

osgConv.o: osgConv.cpp
	g++ $(CXX) -c osgConv.cpp $(incPaths)

CreateExamplePictures.exe: CreateExamplePictures.o Utils.o
	g++ $(CXX) -o CreateExamplePictures.exe CreateExamplePictures.o $(incPaths) $(libPathOSG) $(libOSG)

CreateExamplePictures.o: CreateExamplePictures.cpp
	g++ $(CXX) -c CreateExamplePictures.cpp $(incPaths)

ColorExtractor.exe: GetDominantColors.o
	g++ $(CXX) -o GetDominantColors.exe GetDominantColors.o $(incPaths) $(libPathOSG) $(libOSG) $(libPathCV) $(libOpenCV)

GetDominantColors.o: GetDominantColors.cpp
	g++ $(CXX) -c GetDominantColors.cpp $(incPaths)

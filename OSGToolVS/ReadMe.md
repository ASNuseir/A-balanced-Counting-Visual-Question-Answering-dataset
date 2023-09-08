Requirements:
OpenSceneGraph 3.5.0 with PNG Plugin
	PNG Plugin requires:
		libpng
		zlib
OpenCV 4.3.0 (only if automated Color Extraction is used)
OpenGL Version  4.6 or Higher, glsl support version 4.6 or higher

To compile main body run:
make MainShell.exe

Other possible targets:
Conv.exe
CreateExamplePictures.exe
ColorExtractor.exe
Testing.exe 

Within the Makefile libPathOSG, libPathCV, and incPaths may have to be adjusted to point to where OSG is installed

It is recommended to move the resulting Executable to a separate Folder with the following structure:

OSGTool
./Images/ - Will contain the generated images
./FloorTilecolors/ - Will contain debug information when running the Color Extractor
target.txt - Contains the paths to the Models and their names
ColorsRGB.txt - Contains the floor tile Colors

Executable Functions:

MainShell.exe:
Runs the Main Programm, generating automated Images + VQA Data from the Parameters and Models

Conv.exe:
Normalizes a given list of images to fit the parameters demanded by MainShell.exe

ColorExtractor.exe:
Extracts the characteristic Color of a given list of Objects and saves it to a file

The following two are utility tools for the creation of this programm/the paper
CreateExamplePictures.exe:
Creates a certain type of Example Picture of the Bachelor's Thesis

Testing.exe:
Displays Debugging information from Conv.exe, if it has been generated
The problem it was created for has been solved already.

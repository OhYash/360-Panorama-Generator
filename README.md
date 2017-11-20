360° Panorama Generator
=======================

*By Yash Yadav, Km Varsha Jaiswal and Anamika Singh*

360° Panorama Generator is a relative images stitcher written in C++. The program takes individual images as input and generates a stitched panorama picture as output. The program supports giving outputs in 5 formats i.e. Plane, cylindrical, spherical, fisheye, and stereographic.

- The program has been developed on linux environment and has been successfully tested on the same. The instructions assume that the user is on any linux-distro or is able to apply the instructions on their respective OS of choice.

- The program has two components : Panorama Stitcher and Cropper. Stitcher joins individual images into one and Cropper crops the stitched image into a uniform rectangle.


**Installation :**

-- Compilation requires g++ compiler and opencv. Open **terminal**, change directory to this repository and enter the following commands :  

	sh ubuntuOpenCV/opencv_latest.sh

**Compiler requirements :**

Compile the two files :

	g++ src/stitcher.cpp -o stitcher `pkg-config --cflags --libs opencv`

	g++ src/cropper.cpp -o cropp `pkg-config --cflags --libs opencv`

**Run the program :**

Run the program like this :  

	./stitcher [image1] [image2] [image3].......[image4]  [flags]

Enter **y** or **n** when the program asks you to crop the images.

To know about the supported flags, simply run without any arguments :  

	./stitcher

**Examples :**   

To combine flowers in the images/flower folder :  
	
	./stitcher images/flower/* --output flower.jpg

or to stitch the images in the UAV foler as a stereographic image :  
	
	./stitcher images/uav/* --warp stereographic --output UAV_stereographic.jpg


Done! Open the image with your image viewer!

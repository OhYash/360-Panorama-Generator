#include <iostream>
#include <fstream>
#include <string>
#include "opencv2/opencv_modules.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/timelapsers.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"

bool checkInteriorExterior(const cv::Mat&mask, const cv::Rect&interiorBB, int&top, int&bottom, int&left, int&right)
{
// return true if the rectangle is fine as it is!
bool returnVal = true;

cv::Mat sub = mask(interiorBB);

unsigned int x=0;
unsigned int y=0;

// count how many exterior pixels are at the
unsigned int cTop=0; // top row
unsigned int cBottom=0; // bottom row
unsigned int cLeft=0; // left column
unsigned int cRight=0; // right column
// and choose that side for reduction where mose exterior pixels occured (that's the heuristic)

for(y=0, x=0 ; x<sub.cols; ++x)
{
    // if there is an exterior part in the interior we have to move the top side of the rect a bit to the bottom
    if(sub.at<unsigned char>(y,x) == 0)
    {
        returnVal = false;
        ++cTop;
    }
}

for(y=sub.rows-1, x=0; x<sub.cols; ++x)
{
    // if there is an exterior part in the interior we have to move the bottom side of the rect a bit to the top
    if(sub.at<unsigned char>(y,x) == 0)
    {
        returnVal = false;
        ++cBottom;
    }
}

for(y=0, x=0 ; y<sub.rows; ++y)
{
    // if there is an exterior part in the interior
    if(sub.at<unsigned char>(y,x) == 0)
    {
        returnVal = false;
        ++cLeft;
    }
}

for(x=sub.cols-1, y=0; y<sub.rows; ++y)
{
    // if there is an exterior part in the interior
    if(sub.at<unsigned char>(y,x) == 0)
    {
        returnVal = false;
        ++cRight;
    }
}

// that part is ugly and maybe not correct, didn't check whether all possible combinations are handled. Check that one please. The idea is to set `top = 1` iff it's better to reduce the rect at the top than anywhere else.
if(cTop > cBottom)
{
    if(cTop > cLeft)
        if(cTop > cRight)
            top = 1;
}
else
    if(cBottom > cLeft)
        if(cBottom > cRight)
            bottom = 1;

if(cLeft >= cRight)
{
    if(cLeft >= cBottom)
        if(cLeft >= cTop)
            left = 1;
}
else
    if(cRight >= cTop)
        if(cRight >= cBottom)
            right = 1;



return returnVal;
}

bool sortX(cv::Point a, cv::Point b)
{
    bool ret = false;
    if(a.x == a.x)
        if(b.x==b.x)
            ret = a.x < b.x;

    return ret;
}

bool sortY(cv::Point a, cv::Point b)
{
    bool ret = false;
    if(a.y == a.y)
        if(b.y == b.y)
            ret = a.y < b.y;


    return ret;
}

int main(int argv, char **argc)
{
	cv::Mat input = cv::imread(argc[1]);

cv::Mat gray;
cv::cvtColor(input,gray,CV_BGR2GRAY);

char cmd[100];

cv::imshow("gray", gray);

// extract all the black background (and some interior parts maybe)
cv::Mat mask = gray>0;
cv::imshow("mask", mask);

// now extract the outer contour
std::vector<std::vector<cv::Point> > contours;
std::vector<cv::Vec4i> hierarchy;

cv::findContours(mask,contours,hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cv::Point(0,0));

cv::Mat contourImage = cv::Mat::zeros( input.size(), CV_8UC3 );;

//find contour with max elements
// remark: in theory there should be only one single outer contour surrounded by black regions!!

unsigned int maxSize = 0;
unsigned int id = 0;
for(unsigned int i=0; i<contours.size(); ++i)
{
    if(contours.at(i).size() > maxSize)
    {
        maxSize = contours.at(i).size();
        id = i;
    }
}

/// Draw filled contour to obtain a mask with interior parts
cv::Mat contourMask = cv::Mat::zeros( input.size(), CV_8UC1 );
cv::drawContours( contourMask, contours, id, cv::Scalar(255), -1, 8, hierarchy, 0, cv::Point() );
cv::imshow("contour mask", contourMask);

// sort contour in x/y directions to easily find min/max and next
std::vector<cv::Point> cSortedX = contours.at(id);
std::sort(cSortedX.begin(), cSortedX.end(), sortX);

std::vector<cv::Point> cSortedY = contours.at(id);
std::sort(cSortedY.begin(), cSortedY.end(), sortY);


unsigned int minXId = 0;
unsigned int maxXId = cSortedX.size()-1;

unsigned int minYId = 0;
unsigned int maxYId = cSortedY.size()-1;

cv::Rect interiorBB;
cv::Mat cropped;
while( (minXId<maxXId)&&(minYId<maxYId) )
{
    cv::Point min(cSortedX[minXId].x, cSortedY[minYId].y);
    cv::Point max(cSortedX[maxXId].x, cSortedY[maxYId].y);

    interiorBB = cv::Rect(min.x,min.y, max.x-min.x, max.y-min.y);

// out-codes: if one of them is set, the rectangle size has to be reduced at that border
    int ocTop = 0;
    int ocBottom = 0;
    int ocLeft = 0;
    int ocRight = 0;

    bool finished = checkInteriorExterior(contourMask, interiorBB, ocTop, ocBottom,ocLeft, ocRight);
    if(finished)
    {
        break;
    }

// reduce rectangle at border if necessary
    if(ocLeft)++minXId;
    if(ocRight) --maxXId;

    if(ocTop) ++minYId;
    if(ocBottom)--maxYId;


}

//std::cout << "Vars :\n ocTop :interiorBB : " << interiorBB.x << std::endl;
cv::Mat mask2 = cv::Mat::zeros(input.rows, input.cols, CV_8UC1);
sprintf(cmd, "convert %s -crop %dx%d+%d+%d +repage %s", argc[1], interiorBB.width, interiorBB.height, interiorBB.x, interiorBB.y, argc[1]);
/*cv::rectangle(mask2,interiorBB, cv::Scalar(255),-1);

cv::Mat maskedImage;
input.copyTo(maskedImage);
for(unsigned int y=0; y<maskedImage.rows; ++y)
    for(unsigned int x=0; x<maskedImage.cols; ++x)
    {
        maskedImage.at<cv::Vec3b>(y,x)[2] = 255;
    }
input.copyTo(maskedImage,mask2);
cv::imshow("masked image", maskedImage);
cv::imwrite("interiorBoundingBoxResult.png", maskedImage);
//cv::imwrite("CroppedImg.png", cropped);*/
cv::Mat roi(input, interiorBB);
cv::Mat croppedImg;
roi.copyTo(croppedImg);

//cv::Rect roi(interiorBB.x, interiorBB.y, interiorBB.width, interiorBB.height);
//cv::Mat croppedImg = cv::fullImage(cv::Rect(interiorBB));
//cv::namedWindow("Display Window", cv::WINDOW_AUTOSIZE);
cv::imwrite(argc[1], croppedImg);
//cv::imshow("Masked Image", croppedImg);
//cv::waitKey(0);
//system(cmd);
std::cout << "Image Cropped!"<<std::endl;
}

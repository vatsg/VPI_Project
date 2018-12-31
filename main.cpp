#include "Sharpening.h"
#include "Contouring.h"
#include "Reconstruction.h"

#include "opencv2/opencv.hpp"
#include <iostream>

#include "Tree.h"
#include "Contour.h"

using namespace std;

cv::Mat sharpening(cv::Mat image) {
    return sharpenImage(image);
}

Tree contouring(cv::Mat image) {
    return encodeImage(image);
}

cv::Mat reconstructing(Tree enclosureTree, cv::Point2i imageShape) {

    return runReconstruction(enclosureTree,imageShape);
}

int main() {

    cv::Mat image;
    std::string IMAGE_FOLDER = "/Users/Govin/Documents/MSCS/VPI_Research/VPI_Code/";
    std::string IMAGE_NAME = "test_image.tiff";
    std::string IMAGE_PATH = IMAGE_FOLDER + IMAGE_NAME;

    image = cv::imread(IMAGE_PATH,CV_LOAD_IMAGE_GRAYSCALE);
    if(!image.data )                              // Check for invalid input
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    image = sharpening(image);
    Tree enclosureTree = contouring(image);

    printf("Done Contouring");

    cv::Point2i imageShape(image.cols, image.rows);
    cv::Mat reconstructedImage = reconstructing(enclosureTree,imageShape);
    printf("\nDone Encoding\n");

//    cv::namedWindow( "window", cv::WINDOW_AUTOSIZE );// Create a window for display.
//    cv::imshow("window",reconstructedImage);
//    cv::waitKey(0);

    cv::imwrite("testOut.jpg",reconstructedImage);
    return 0;
}

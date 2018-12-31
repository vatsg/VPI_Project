//
// Created by Govin Vatsan on 7/18/17.
//
// Represents a "Contour" object. This object contains the points that forms the contour, the
// level of the contour in the image, the boundaries of a rectangle enclosing the contour,
// and the area of the contour (approx. by Green's theorem)
//

#ifndef C_CODE_CONTOUR_H
#define C_CODE_CONTOUR_H

#include <iostream>
#include <vector>
#include <array>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class Contour {
    private:
        double contourLevel;
        vector<Point2f> crossingPoints;
        array<double, 4> boundingRectangle;
        double contourArea;

    public:
        Contour(double level, vector<Point2f> crossPts, array<double, 4> bounds, double area);
        double getLevel();
        vector<Point2f> getPoints();
        array<double, 4> getBoundingRectangle();
        double getContourArea();
};



#endif //C_CODE_CONTOUR_H

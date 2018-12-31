//
// Created by Govin Vatsan on 7/18/17.
//

#include "Contour.h"

Contour::Contour(double level, vector<Point2f> crossPts, array<double, 4> bounds, double area){
    contourLevel = level;
    crossingPoints = crossPts; // (row, col)
    boundingRectangle = bounds; // of the form (rowMin, rowMax, colMin, colMax)
    contourArea = area;
}

double Contour::getLevel() {
    return contourLevel;
}

vector<Point2f> Contour::getPoints() {
    return crossingPoints;
}

array<double, 4> Contour::getBoundingRectangle() {
    return boundingRectangle;
}

double Contour::getContourArea() {
    return contourArea;
}


//
// Created by Govin Vatsan on 7/18/17.
//

#include "Reconstruction.h"


using namespace std;
using namespace cv;

vector<Point2i> convertFloatToInt(vector<Point2f> inputPts) {

    vector<Point2i> outputPts = {};

    for (int i = 0; i < inputPts.size(); i++) {
        int y = (int)(inputPts[i].y + 0.5);
        int x = (int)(inputPts[i].x + 0.5);

        outputPts.push_back(Point2i(x,y));
    }

    return outputPts;
}


int findMinHeight(Point2i currLoc, Mat& matrix) {

    int numRows = matrix.rows;
    int numCols = matrix.cols;

    int minHeight = (int) matrix.at<uchar>(currLoc.y, currLoc.x);

    if ((currLoc.y - 1) >= 0 ){
        int newVal = (int) matrix.at<uchar>(currLoc.y-1, currLoc.x);
        minHeight = min(newVal, minHeight);
    }
    if ((currLoc.y + 1) < numRows) {
        int newVal = (int) matrix.at<uchar>(currLoc.y+1, currLoc.x);
        minHeight = min(newVal, minHeight);
    }
    if ((currLoc.x-1) >= 0) {
        int newVal = (int) matrix.at<uchar>(currLoc.y, currLoc.x-1);
        minHeight = min(newVal, minHeight);
    }
    if ((currLoc.x+1) < numCols) {
        int newVal = (int) matrix.at<uchar>(currLoc.y, currLoc.x+1);
        minHeight = min(newVal, minHeight);
    }

    return minHeight;
}

void interpolationFill(Contour parentContour, vector<Contour> childrenContours, Mat* image) {
    /*
     * Interpolation Fill Algorithm
     */

    int numChildren = childrenContours.size();

    // determine each unique contourLevel amongst parent + children (max of 2)
    set<double> uniqueContourLevels; // use this to detect levels, IF ONLY 1 CONTOUR LEVEL, just use standard fill!
    uniqueContourLevels.insert(parentContour.getLevel());
    for (int i = 0; i < numChildren; i++) {
        double currChildLevel = childrenContours[i].getLevel();
        uniqueContourLevels.insert(currChildLevel); // will only insert unique vals
    }

    // Check if there are no child contours or if only one unique contourLevel. If so, use a standard polygon fill
    if (childrenContours.size()==0 || uniqueContourLevels.size()==1){
        vector<Point2i> parentContourPts = convertFloatToInt(parentContour.getPoints()); // need to get points in int, not floats

        const Point* firstPt = &(parentContourPts[0]); // we can just use the parent to fill up the contour
        int numPts = parentContourPts.size();
        cv::fillPoly(*image, &firstPt,&numPts,1,Scalar(parentContour.getLevel())); // uses OpenCV to apply standard polygon fill
    }
    else if (uniqueContourLevels.size() == 2) { // need to use interpolation fill
        // create two grids, one for each height -- TODO: reduce size of grid to relevant pixels

        double level1 = *uniqueContourLevels.begin(); // get both contour levels
        double level2 = *prev(uniqueContourLevels.end());

        Mat grid1(image->rows, image->cols, CV_8U); // grid for level1
        Mat grid2(image->rows, image->cols, CV_8U); // grid for level2
        grid1 = Scalar(254);
        grid2 = Scalar(254); // fill grids to 254 - default value (for a 8U image)

        Mat tempGrid(image->rows, image->cols, CV_8U); // temp grid to store all pixels that will be affected by the interpolation
        tempGrid = Scalar(0); // set all values to 0

        for (int i = -1; i < numChildren; i++) { // iterate over all contours (including parent)
            Contour *currContour;
            if (i == -1) {
                currContour = &parentContour;
            } else {
                currContour = &(childrenContours[i]);
            }

            vector<Point2i> contourPts = convertFloatToInt(currContour->getPoints()); // need to get points in int, not floats
            if (i == -1) {
                const Point *firstPt = &(contourPts[0]);
                int numPts = contourPts.size();
                cv::fillPoly(tempGrid, &firstPt, &numPts, 1, Scalar(255)); // fill tempGrid with px vals inside parent Contour
            }

            // on grid2, all contours of height level1 = 255, on grid1, contours of height level1 = 0
            if (currContour->getLevel() == level1) {
                for (int j = 0; j < contourPts.size(); j++) {
                    int currPoint = (int) grid2.at<uchar>(contourPts[j].y, contourPts[j].x);
                    if (currPoint !=
                        0) {  // NOTE: NEVER OVERWRITE A ROOT CONTOUR WITH A BOUNDARY CONTOUR (so if it's already = 0, don't overwrite)
                        grid2.at<uchar>(contourPts[j].y, contourPts[j].x) = 255;
                    }

                    grid1.at<uchar>(contourPts[j].y, contourPts[j].x) = 0;
                }
            } else { // on grid1, all contours of height level2 = 255, on grid2, contours of height level2 = 0
                for (int j = 0; j < contourPts.size(); j++) {
                    int currPoint = (int) grid1.at<uchar>(contourPts[j].y, contourPts[j].x);
                    if (currPoint != 0) {
                        grid1.at<uchar>(contourPts[j].y, contourPts[j].x) = 255;
                    }

                    grid2.at<uchar>(contourPts[j].y, contourPts[j].x) = 0;
                }
            }
        }

        int rootVal = 0;
        int boundaryVal = 255;
        int rMin = (int) parentContour.getBoundingRectangle()[0];
        int rMax = (int) (parentContour.getBoundingRectangle()[1]+1);
        int cMin = (int) parentContour.getBoundingRectangle()[2];
        int cMax = (int) (parentContour.getBoundingRectangle()[3] + 1);

        for (int c = 0; c < 2;c++) {
            int currLevel;
            Mat *currGrid;
            if (c == 1){
                currLevel = level1;
                currGrid = &grid1;
            }
            else{
                currLevel = level2;
                currGrid = &grid2;
            }
            bool changesMade = true;

            while (changesMade) {
                changesMade = false;

                for (int r = rMin; r< rMax; r++){
                    for (int c = cMin; c<cMax; c++){
                        int tempVal = (int) tempGrid.at<uchar>(r, c);

                        if (tempVal == 255) { // if pixel is inside parent contour
                            int currVal = (int) currGrid->at<uchar>(r,c);
                            if (currVal == rootVal){
                                // do nothing
                            }
                            else if (currVal == boundaryVal){
                                // do nothing
                            }
                            else {
                                int minHeight = findMinHeight(Point2i(c,r),*currGrid);
                                if (minHeight < (boundaryVal-1)){
                                    if(currVal != (minHeight+1)){
                                        changesMade = true;
                                        currGrid->at<uchar>(r, c) = minHeight+1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now apply final polygon fill to grids
        for (int r = rMin; r< rMax; r++) {
            for (int c = cMin; c < cMax; c++) {
                int tempVal = (int) tempGrid.at<uchar>(r, c);
                if (tempVal == 255){

                    int manhattanDist1 = (int) grid1.at<uchar>(r,c);
                    int manhattanDist2 = (int) grid2.at<uchar>(r,c);

                    double heightValNumer = 0.0;
                    double heightValDenom = 0.0;

                    bool isRootContour = false;

                    if (manhattanDist1 == 0){ // it's a contour
                        image->at<uchar>(r,c) = (int) level1;
                        isRootContour = true;
                        break;
                    }
                    else if (manhattanDist1 != 0) { // calculate weighted height value from manhattan distances
                        double dist = manhattanDist1;
                        heightValNumer += level1/dist;
                        heightValDenom += 1/dist;
                    }

                    if (manhattanDist2 == 0){
                        image->at<uchar>(r,c) = (int) level2;
                        isRootContour = true;
                        break;
                    }
                    else if (manhattanDist2 !=0){
                        double dist = manhattanDist2;
                        heightValNumer += level2/dist;
                        heightValDenom += 1/dist;
                    }

                    if (isRootContour == false){ // if current pixel is not on a contour
                        int newHeight = (int) (heightValNumer/heightValDenom);
                        image->at<uchar>(r,c) = newHeight;
                    }
                }
            }
        }
    }
    else {
        printf("Error. Reconstruction, more than 2 unique contour levels.\n");
    }

//        map<double, vector<Contour *>> mapContoursByLevel; // arranges contours by their contour levels
//        mapContoursByLevel[parentContour.getLevel()];
//        mapContoursByLevel[parentContour.getLevel()].push_back(&parentContour); // add pointer to parentContour to corresponding level
//
//        for (int i = 0; i < numChildren; i++){
//            double currChildLevel = childrenContours[i].getLevel();
//            Contour* pointerToChildContour = &(childrenContours[i]);
//
//   mapContoursByLevel[currChildLevel] = {pointerToChildContour};
//            }
//            else { // contour level is in map
//                mapContoursByLevel[currChildLevel].push_back(pointerToChildContour);
//            }
//        }

        // Set value at other contours (not the current height) to 255 - these are the boundary contours
//        for

//    }
}

void polygonFill(Contour parentContour, vector<Contour> childrenContours, Mat* image){
    /*
     * Polygon Fill Algorithm - from OpenCV
     *
     */

    vector<Point2f> pointsF = parentContour.getPoints();
    vector<Point2i> pointsI = {};

    for (int i = 0; i< pointsF.size();i++){
        int row = (int)(pointsF[i].y + 0.5);
        int col = (int)(pointsF[i].x + 0.5);

        pointsI.push_back(Point2i(col,row));
//        image->at<uchar>(row, col) = parentContour.getLevel();
    }
    const Point* p = &(pointsI[0]);
    int numPts = pointsI.size();
    cv::fillPoly(*image, &p,&numPts,1,Scalar(parentContour.getLevel()));

}

void reconstructImage(Tree enclosureTreeRoot, Mat* image) {

    queue<Tree> treeQueue; // queue to process all contours in FIFO order

    for (int i =0; i < enclosureTreeRoot.getNumChildren(); i++) {
        treeQueue.push(enclosureTreeRoot.children[i]); // add child to queue
    }

    // Loop through each root-child relationship and interpolate the colors between them
    while (treeQueue.size() != 0){
        Tree currTreeNode = treeQueue.front(); // access first contour from queue (need to deference)
        treeQueue.pop(); // pops off the returned contour from queue (treeQueue.front() does not remove Contour from queue)

        vector<Contour> childrenContours;
        for (int i = 0; i < currTreeNode.getNumChildren(); i++) {
            childrenContours.push_back(currTreeNode.children[i].getContour()); // gets child contours from currTreeNode
            treeQueue.push(currTreeNode.children[i]); // pushes child Tree onto queue
        }

        interpolationFill(currTreeNode.getContour(), childrenContours, image); // call interpolation fill with curr-contour + child-contours
    }
}

Mat runReconstruction(Tree enclosureTree, Point2i imageShape) {
    /*
     * Takes in an enclosure tree and the image shape to reconstruct the original image by diffusing between
     * contours
     *
     * param: enclosureTree
     *        imageShape
     *
     * output: image - cv::Mat object, reconstructed image
     */

    Mat image = Mat::zeros(imageShape.y, imageShape.x, CV_8U); // creates a new 2D matrix of zeros
    reconstructImage(enclosureTree, &image); // runs reconstruction process on image
    return image;
}
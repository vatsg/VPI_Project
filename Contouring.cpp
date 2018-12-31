//
// Created by Govin Vatsan on 7/18/17.
//

#include "Contouring.h"

using namespace std;
using namespace cv;

// Global Variables
double IMAGE_MIN = 0;
double IMAGE_MAX = 0;
double Y_INTERVAL = 10;


double getCrossingDistance(Mat image, Point2i point1, Point2i point2, double contourLevel) {
    /*
     * Gets the distance between two points that a contour will cross
     * Returns -1 if contour will not cross between those two points
     */
    int val1 = image.at<uchar>(point1.y, point1.x); // note: Point2i is of form (x,y) = (col,row)
    int val2 = image.at<uchar>(point2.y, point2.x);

    if (min(val1, val2) <= contourLevel && contourLevel <= max(val1,val2)){
        double valueDiff = abs(val1 - val2);
        double contourDiff = abs(val1 - contourLevel);

        double crossingDistance = contourDiff / valueDiff;
        return crossingDistance;
    }
    else {
        return -1;
    }
}

vector<Point2f> findCrossingPoints(Mat image, int row, int col, double contourLevel) {
    /*
     * Finds all of the linearly interpolated crossing points of the contour over the current pixel-box
     * returns: crossingPoints: array of points where a contour crosses a pixel box (row, col)
     *
     * Note: The order of crossing points is very important. Order is: Left Col, Top Row, Bottom Row, Right Col. This ensures each
     * contour segment will have a "direction" from a lower coordinate pixel to a higher coordinate pixel.
     */

    vector<Point2f> crossingPoints; // points where the contour crosses the pixel-box

    double crossingDist; // stores the point at which a contour will cross along a pixel-box (if valid)
    Point2f crossingPoint(0,0); // keeps track of individual crossing points over a px-box

    // Left Col
    Point2i point1(col,row);
    Point2i point2(col,row+1);

    crossingDist = getCrossingDistance(image, point1, point2, contourLevel);
    if (crossingDist != -1) {
        crossingPoint.y = row + crossingDist; crossingPoint.x = col;
        crossingPoints.push_back(crossingPoint);
    }

    // Top Row
    point1.y = row; point1.x = col;
    point2.y = row; point2.x = col+1;
    crossingDist = getCrossingDistance(image, point1, point2, contourLevel);
    if (crossingDist != -1) {
        crossingPoint.y = row; crossingPoint.x = col+crossingDist;
        crossingPoints.push_back(crossingPoint);
    }

    // Bottom Row
    point1.y = row+1; point1.x = col;
    point2.y = row+1; point2.x = col+1;
    crossingDist = getCrossingDistance(image, point1, point2, contourLevel);
    if (crossingDist != -1) {
        crossingPoint.y = row+1; crossingPoint.x = col+crossingDist;
        crossingPoints.push_back(crossingPoint);
    }

    // Right Col
    point1.y = row; point1.x = col+1;
    point2.y = row+1; point2.x = col+1;
    crossingDist = getCrossingDistance(image, point1, point2, contourLevel);
    if (crossingDist != -1) {
        crossingPoint.y = row + crossingDist; crossingPoint.x = col+1;
        crossingPoints.push_back(crossingPoint);
    }

    return crossingPoints;
}

bool ptEquals(double a, double b) {
    /*
     * Compares 2 points for equality (within a certain range)
     */

    if (a == b) {
        return true;
    }
    return false;
}


map<double, vector<vector<Point2f>>> dominoContouring(cv::Mat image) {
    /*
     * Finds all contours for the image
     * Returns: contourList: HashMap
     */

    map<double, vector<vector<Point2f>>> contourList; // final map that stores completed contours

    for(double contourLevel = IMAGE_MIN + 0.5; contourLevel< IMAGE_MAX - 0.5; contourLevel += Y_INTERVAL) {
        contourList[contourLevel]; // initialize this key on the contourList
        vector<vector<Point2f>> unfinishedContours = {}; // stores all of the unfinished contours found

        int row = 0; // curr row
        int col = 0; // curr col

        while (row < (image.rows - 1)){
            vector<Point2f> crossingPoints = findCrossingPoints(image, row, col, contourLevel); // finds contour pts crossing box
            // Note: Above vector must have either two or four pairs. 2 means there's one crossing segment, 4 means there's two segments

            for (int crossPtIter = 0; crossPtIter < crossingPoints.size(); crossPtIter+=2) { // increments by 2 to iterate over two points (or one line segment)
                Point2f segmentStartPoint = crossingPoints[crossPtIter];
                Point2f segmentEndPoint = crossingPoints[crossPtIter+1];

                bool segmentAdded = false; // tracks whether the segment has been added to a contour yet
                int currContourIndex = 0; // stores index of contour that curr segment is added to

                for (int unfContourIter = 0; unfContourIter < unfinishedContours.size(); unfContourIter++) {
                    vector<Point2f> currUnfContour = unfinishedContours[unfContourIter];
                    Point2f contourStartPoint = currUnfContour[0];
                    Point2f contourEndPoint = currUnfContour[currUnfContour.size()-1];

                    if (ptEquals(contourEndPoint.y,segmentStartPoint.y) && ptEquals(contourEndPoint.x,segmentStartPoint.x)) {
                        unfinishedContours[unfContourIter].push_back(segmentEndPoint);
                        segmentAdded = true;
                    }
                    else if (ptEquals(contourStartPoint.y,segmentStartPoint.y) && ptEquals(contourStartPoint.x,segmentStartPoint.x)) {
                        unfinishedContours[unfContourIter].insert(unfinishedContours[unfContourIter].begin(),segmentEndPoint);
                        segmentAdded = true;
                    }
                    else if (ptEquals(contourStartPoint.y,segmentEndPoint.y) && ptEquals(contourStartPoint.x,segmentEndPoint.x)) {
                        unfinishedContours[unfContourIter].insert(unfinishedContours[unfContourIter].begin(),segmentStartPoint);
                        segmentAdded = true;
                    }
                    else if (ptEquals(contourEndPoint.y,segmentEndPoint.y) && ptEquals(contourEndPoint.x,segmentEndPoint.x)) {
                        unfinishedContours[unfContourIter].push_back(segmentStartPoint);
                        segmentAdded = true;
                    }

                    if (segmentAdded == true) {
                        currContourIndex = unfContourIter;
                        break; // no need to loop through all active contours if segment has been added
                    }
                }
                if (segmentAdded == true){ // need to check if the added segment completes the contour or joins two contours
                    vector<Point2f> currUnfContour = unfinishedContours[currContourIndex];
                    Point2f currContourStart = currUnfContour[0];
                    Point2f currContourEnd = currUnfContour[currUnfContour.size()-1];

                    if (ptEquals(currContourStart.y,currContourEnd.y) && ptEquals(currContourStart.x,currContourEnd.x)) {
                        contourList[contourLevel].push_back(currUnfContour);

                        unfinishedContours[currContourIndex] = unfinishedContours.back(); // assign elt to delete to last elt
                        unfinishedContours.pop_back(); // delete last elt from vector
                    }
                    else {
                        int index_startCurrContour_matches_endOtherContour = -1;
                        int index_startCurrContour_matches_startOtherContour = -1;
                        int index_endCurrContour_matches_endOtherContour = -1;
                        int index_endCurrContour_matches_startOtherContour = -1;

                        for (int unfContourIter = 0; unfContourIter < unfinishedContours.size(); unfContourIter++) {
                            if (unfContourIter != currContourIndex) { // need to skip over current contour
                                Point2f otherContourStart = unfinishedContours[unfContourIter][0];
                                Point2f otherContourEnd = unfinishedContours[unfContourIter][unfinishedContours[unfContourIter].size()-1];

                                if (ptEquals(currContourStart.y,otherContourEnd.y) && ptEquals(currContourStart.x,otherContourEnd.x)) {
                                    index_startCurrContour_matches_endOtherContour = unfContourIter;
                                }
                                else if (ptEquals(currContourStart.y,otherContourStart.y) && ptEquals(currContourStart.x,otherContourStart.x)) {
                                    index_startCurrContour_matches_startOtherContour = unfContourIter;
                                }
                                else if (ptEquals(currContourEnd.y,otherContourEnd.y) && ptEquals(currContourEnd.x,otherContourEnd.x)) {
                                    index_endCurrContour_matches_endOtherContour = unfContourIter;
                                }
                                else if (ptEquals(currContourEnd.y,otherContourStart.y) && ptEquals(currContourEnd.x,otherContourStart.x)) {
                                    index_endCurrContour_matches_startOtherContour = unfContourIter;
                                }
                            }
                        }

                        vector<Point2f> newContour; // will store the combined contour from existing contours
                        int matchingIndex = -1; // temp var to shorten long index var names

                        // Case 1
                        if (index_startCurrContour_matches_endOtherContour != -1) {
                            matchingIndex = index_startCurrContour_matches_endOtherContour; // just shortening the var name

                            newContour = unfinishedContours[matchingIndex];
                            newContour.insert(newContour.end(), currUnfContour.begin()+1,currUnfContour.end());
                        }
                        // Case 2
                        else if (index_startCurrContour_matches_startOtherContour != -1) {
                            matchingIndex = index_startCurrContour_matches_startOtherContour;

                            newContour = unfinishedContours[matchingIndex];
                            reverse(newContour.begin(),newContour.end());
                            newContour.insert(newContour.end(), currUnfContour.begin() +1, currUnfContour.end());
                        }
                        // Case 3
                        else if (index_endCurrContour_matches_endOtherContour != -1) {
                            matchingIndex = index_endCurrContour_matches_endOtherContour;

                            newContour = unfinishedContours[matchingIndex];
                            reverse(currUnfContour.begin(), currUnfContour.end());
                            newContour.insert(newContour.end(),currUnfContour.begin()+1,currUnfContour.end());
                        }
                        // Case 4
                        else if (index_endCurrContour_matches_startOtherContour != -1) {
                            matchingIndex = index_endCurrContour_matches_startOtherContour;

                            newContour = currUnfContour;
                            newContour.insert(newContour.end(),unfinishedContours[matchingIndex].begin()+1, unfinishedContours[matchingIndex].end());
                        }

                        if (matchingIndex != -1){
                            // need to remove matchingIndex, currContourIndex from unfinishedContours array
                            // remove greater index first
                            unfinishedContours[max(matchingIndex, currContourIndex)] = unfinishedContours.back();
                            unfinishedContours.pop_back();

                            unfinishedContours[min(matchingIndex, currContourIndex)] = unfinishedContours.back();
                            unfinishedContours.pop_back();

                            // check if newContour is a complete loop
                            if (ptEquals(newContour[0].y,newContour[newContour.size()-1].y) && ptEquals(newContour[0].x,newContour[newContour.size()-1].x)) {
                                if (newContour[0].x == newContour[newContour.size()-1].x) {
                                    contourList[contourLevel].push_back(newContour); // if it is, add to contourList
                                }
                            }
                            else {
                                unfinishedContours.push_back(newContour);
                            }
                        }

                        // ERROR CHECK
                        int numMatches = 0;
                        if (index_endCurrContour_matches_startOtherContour != -1) {
                            numMatches += 1;
                        }
                        if (index_startCurrContour_matches_endOtherContour != -1) {
                            numMatches += 1;
                        }
                        if (index_startCurrContour_matches_startOtherContour != -1) {
                            numMatches+=1;
                        }
                        if (index_endCurrContour_matches_endOtherContour != -1) {
                            numMatches += 1;
                        }
                        if (numMatches > 1) {
                            printf("Multiple Matches. CHECK ASAP.");
                        }
                    }
                }
                else { // need to create a new active contour list
                    vector<Point2f> newContour = {segmentStartPoint, segmentEndPoint};
                    unfinishedContours.push_back(newContour);
                }
            }

            col+=1; // move to next px-box in the row
            if (col == (image.cols - 1)) { // move to next row
                col = 0;
                row +=1;
            }
        }

        if (unfinishedContours.size() != 0) {
            printf("Unfinished Contours Size != 0. Error.\n");
        }
    }

    return contourList;
}

array<double, 4> calcBoundingRectangle(vector<Point2f> contourPts) {
    /*
     * return: bounding rectangle of the form (rowMin, rowMax, colMin, colMax)
     */

    double rowMin = contourPts[0].y;
    double rowMax = contourPts[0].y;
    double colMin = contourPts[0].x;
    double colMax = contourPts[0].x;

    for (int i = 1; i < contourPts.size(); i++) {
        if (contourPts[i].y < rowMin) {
            rowMin = contourPts[i].y;
        }
        if (contourPts[i].y > rowMax) {
            rowMax = contourPts[i].y;
        }
        if (contourPts[i].x < colMin ) {
            colMin = contourPts[i].x;
        }
        if (contourPts[i].x > colMax) {
            colMax = contourPts[i].x;
        }
    }

    array<double, 4> bounds = {rowMin, rowMax, colMin, colMax};
    return bounds;
}

double calcContourArea(vector<Point2f> contourPts) {
    return contourArea(contourPts, false); // openCV contourArea func: params (points, oriented)
}


bool compareBoundingRectangles(Contour A, Contour B){
    /*
     * Returns true if bounding rectangle of a encloses bounding rectangle of b
     */

    array<double, 4> boundsA = A.getBoundingRectangle(); // of form (rowMin, rowMax, colMin, colMax)
    array<double, 4> boundsB = B.getBoundingRectangle();

    if ((boundsA[0] <  boundsB[0]) && (boundsA[1]>boundsB[1])) { // should this be <=, >=?
        if ((boundsA[2] < boundsB[2]) && (boundsA[3] > boundsB[3])){
            return true;
        }
    }
    return false;
}

bool compareContourAreas(Contour A, Contour B) {
    /*
     * Returns true if contour A has a larger area than contour B, false otherwise
     */

    return A.getContourArea() >= B.getContourArea();
}

bool pointInPolygonTest (Contour A, Contour B) {
    /*
     * Returns true if contour A encloses a point on contour B, false otherwise
     */

    Point2f testPoint(B.getPoints()[0].y,B.getPoints()[0].x);
    int retVal = pointPolygonTest(A.getPoints(),testPoint,false);

    if (retVal >= 0) {
        return true;
    }
    else {
        return false;
    }
}

bool isContourEnclosed (Contour A, Contour B){
    /*
     * Returns true if A encloses B, false otherwise
     */

    return (compareBoundingRectangles(A,B) && compareContourAreas(A,B) && pointInPolygonTest(A,B));
}


map<double,vector<Contour>> createContourDictionary(map<double, vector<vector<Point2f>>> contourList) {

    map<double, vector<Contour>> contourDict;
    for (double contourLevel = IMAGE_MIN + 0.5; contourLevel < IMAGE_MAX - 0.5; contourLevel += Y_INTERVAL) {
        contourDict[contourLevel]; // initialize an  array for this key on the map

        for (int i = 0; i < contourList[contourLevel].size(); i++) {

            vector<Point2f> contourPts = contourList[contourLevel][i];
            array<double, 4> contourBounds = calcBoundingRectangle(contourPts);
            double contourArea = calcContourArea(contourPts);

            contourDict[contourLevel].push_back(Contour(contourLevel, contourPts, contourBounds, contourArea)); // add contour to dict
        }
    }

    return contourDict;
}

Tree createEnclosureTree(map<double,vector<Contour>> contourDict, Point2i imageDimensions) {
    /*
        Use the following approach to create a contour tree: (insertion sort variant)
        1. The root is always the outer bounding contour (dimensions of the image)
        2. Compare each contour in turn.
        3. For each contour, check the new contour with each of the root's daughters
        4. If it is enclosed inside the daughter then descend further.
        5. It it encloses the daughter then the new contour becomes a daughter,
            the enclosed daughter is moved down a level to inside the new contour,
            check the new contour with other daughters because they might also be enclosed
        6. If neither then added contour as a new daughter
     */
    double numRows = imageDimensions.y;
    double numCols = imageDimensions.x;
    int area = numRows * numCols;

    vector<Point2f> crossPts = {Point2f(0,0),Point2f(numCols,0),Point2f(numCols,numRows),Point2f(0,numRows)};
    array<double, 4> bounds = {0, numRows, 0, numCols};
    Contour boundingContour(-1, crossPts, bounds, area);

    Tree root(boundingContour); // root node of Tree

    for(double contourLevel = IMAGE_MIN + 0.5; contourLevel< IMAGE_MAX - 0.5; contourLevel += Y_INTERVAL) { // iterate over each contour level
        vector<Contour> contourList = contourDict[contourLevel];
        for (int i = 0; i < contourList.size();i++) { // iterate over all contours at that level
            Contour contour(contourList[i]); // current Contour

            bool isContourAdded = false; // will let us determine when to break the loop
            Tree* currParent = &root; // points to the current parent node

            while (!isContourAdded) { // need to determine where to add the contour into the tree
                int numChildren = currParent->getNumChildren();
                if (numChildren == 0) {
                    currParent->addChild(contour);
                    isContourAdded = true;
                }
                else {
                    for (int j = 0; j < numChildren; j++) {
                        Tree* currDaughter = &((currParent->children)[j]);
                        if (isContourEnclosed(contour, currDaughter->contour)) { // if contour encloses curr child contour
                            Tree newContour(contour);
                            newContour.addChild(*currDaughter);
                            (currParent->children)[j] = newContour;

                            isContourAdded = true;
                        }
                        else if (isContourEnclosed(currDaughter->contour, contour)) { // if curr child contour encloses contour
                            currParent = currDaughter;
                            break;
                        }

                        // If we are on the last contour in daughters and the contour still hasn't been added
                        if (j == (numChildren - 1) && (!isContourAdded)) {
                            currParent->addChild(contour); // add contour as daughter of currParent
                            isContourAdded = true;
                        }
                    }
                }
            }
        }
    }

    set<double> uniqueContourLevels;
    for (int k = 0; k < (root.getNumChildren());k++){
        uniqueContourLevels.insert((root.children[k]).getContour().getLevel());
    }
    if (uniqueContourLevels.size() > 2){
        int a =5;
    }


    return root;

}

void prepareGlobalVars(cv::Mat image) {
    /*
     * Runs one-time pre-processing functions on the image
     */

    cv::minMaxLoc(image, &IMAGE_MIN, &IMAGE_MAX, NULL, NULL, cv::Mat()); // gets max and min intensity values from image


}


Tree encodeImage(cv::Mat image) {
/*
 * Encodes an image into its contour-based representation
 */
    prepareGlobalVars(image); // runs pre-processing functions on image
    map<double, vector<vector<Point2f>>> contourList = dominoContouring(image); // gets all the contours
    map<double,vector<Contour>> contourDict = createContourDictionary(contourList); // creates Contour objects for each contour

    Point2i imageDimensions(image.cols, image.rows);

    Tree enclosureTree = createEnclosureTree(contourDict, imageDimensions);
    return enclosureTree;
}




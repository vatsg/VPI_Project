//
// Created by Govin Vatsan on 7/18/17.
//

#ifndef C_CODE_RECONSTRUCTION_H
#define C_CODE_RECONSTRUCTION_H

#include "opencv2/opencv.hpp"
#include "Tree.h"
#include "Contour.h"

#include <iostream>
#include <vector>
#include <array>



Mat runReconstruction(Tree enclosureTree, Point2i imageShape);


#endif //C_CODE_RECONSTRUCTION_H

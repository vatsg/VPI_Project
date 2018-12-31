//
// Created by Govin Vatsan on 7/19/17.
//

#ifndef C_CODE_TREE_H
#define C_CODE_TREE_H

#include "Contour.h"
#include <vector>

class Tree {
    public:
        Tree(Contour c);
        Contour contour; // stores the current Contour
        vector<Tree> children; // stores all children contours of current Contour

        void addChild(Contour val);
        void addChild(Tree tree);
        int getNumChildren();
        Contour getContour();
};


#endif //C_CODE_TREE_H
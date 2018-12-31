//
// Created by Govin Vatsan on 7/19/17.
//

#include "Tree.h"


Tree::Tree(Contour c):contour(c) {}

void Tree::addChild(Contour val) {
    children.push_back(Tree(val));
}

void Tree::addChild(Tree tree){
    children.push_back(tree);
}

int Tree::getNumChildren(){
    return children.size();
}

Contour Tree::getContour(){
    return contour;
}
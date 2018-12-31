//
// Created by Govin Vatsan on 7/18/17.
//

#include "Sharpening.h"

cv::Mat sharpenImage(cv::Mat image) {

    int border = 1;

    cv::copyMakeBorder(image, image, border, border, border, border, cv::BORDER_REPLICATE);
    cv::copyMakeBorder(image, image, border, border, border, border, cv::BORDER_CONSTANT,0);

    return image;
}
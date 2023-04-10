#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
#include "image.hpp"
#include "lsc.hpp"
#include "const.hpp"

int main()
{
    Image image;
    image.loadImage(image);
    LSC lsc;
    cv::Mat img(image.height, image.width, CV_8UC4, image.input.data());
    cv::cvtColor(img, img, cv::COLOR_RGB2HSV);
    lsc.genValues(image, img);
    lsc.saveValues(image);
    image.blocks.clear();
    lsc.loadValues(image);
    lsc.applyValues(image, img);

    return 0;
}
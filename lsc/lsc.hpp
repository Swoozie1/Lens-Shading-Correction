#pragma once
#include "../image.hpp"
#include <opencv2/opencv.hpp>
struct LSC
{
    // move in Image
    void genValues(Image &image, cv::Mat &img);
    void saveValues(const Image &image);
    void loadValues(Image &image);
    void applyValues(Image &image, cv::Mat &img);
};
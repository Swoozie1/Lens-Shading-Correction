#pragma once
#include "../image.hpp"
#include <opencv2/opencv.hpp>
struct LSC
{
    // move in Image
    void genBlockCoefficients(Image &image, cv::Mat &img);
    void saveBlockCoefficients(const Image &image);
    void loadBlockCoefficients(Image &image);
    void applyBlockCoefficients(Image &image, cv::Mat &img, const char *filename);
};
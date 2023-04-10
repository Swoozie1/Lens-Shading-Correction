#pragma once
#include <vector>
#include <iostream>
namespace cv
{
    struct Mat;
}

struct Pixel
{
    Pixel(unsigned char red, unsigned char green, unsigned char blue) : e{red, green, blue, 255} {}

    uint8_t e[4];
};

class Image
{
public:
    std::vector<Pixel> input;
    std::vector<float> blocks;
    std::vector<uint8_t> dataBuffer;

    int width = 0;
    int height = 0;
    int channels = 0;

    int pixelMaxValue = 0;
    int blockWidth;
    int blockHeight;
    int requiredPixelsWidth;
    int requiredPixelsHeight;
    float averageBrightness;

    void fillImageData(Image &image);
    int getNormalizedvalues(Image &image);
    void genPixelValues(Image &image, cv::Mat &img, const int blockX, const int blockY);
    void applyPixelValues(Image &image, cv::Mat &img, const int &posX, const int &posY, const int &blockWidth, const int &blockHeight);
    void loadImage(Image &image);
    void valuesForFirstSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image);
    void valuesForSecondSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image);
    void valuesForThirdSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image);
    void valuesForFourthSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image);
    void calculate(Image &image, cv::Mat &img, int &x, int &y, int j, int i, float &value1, float &value2, float &value3, float &value4);
};
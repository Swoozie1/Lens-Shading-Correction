#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stbi_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi_image_write.h"
#include "stbi_image_resize.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
const int blocksForWidth = 17;
const int blocksForHeight = 13;

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
};

struct LSC
{
    // move in Image
    void genValues(Image &image, cv::Mat &img);
    void saveValues(const Image &image);
    void loadValues(Image &image);
    void applyValues(Image &image, cv::Mat &img);
};

void fillImageData(Image &image)
{
    size_t offset = 0;
    image.blockHeight = image.height / blocksForHeight;
    image.blockWidth = image.width / blocksForWidth;
    image.requiredPixelsHeight = image.height % blocksForHeight;
    image.requiredPixelsWidth = image.width % blocksForWidth;
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
    for (int i = 0; i < image.width * image.height; i++)
    {
        r = image.dataBuffer[offset + 0];
        g = image.dataBuffer[offset + 1];
        b = image.dataBuffer[offset + 2];
        image.input.push_back({r, g, b});
        offset += 3;
    }
}
int getNormalizedvalues(Image &image)
{
    int max = 0;
    for (int i = 1; i < image.blocks.size(); i++)
    {
        int a = image.blocks[i];
        int b = image.blocks[i - 1];
        if (a > b && a > max)
        {
            max = a;
        }
    }
    return max;
}
void genPixelValues(Image &image, cv::Mat &img, const int blockX, const int blockY)
{
    float blockBrightness = 0;
    for (int i = 0; i < image.blockHeight; i++)
    {
        for (int j = 0; j < image.blockWidth; j++)
        {
            int x = (j + (blockX * image.blockWidth));
            int y = (i + (blockY * image.blockHeight));
            cv::Vec3b pixel = img.at<cv::Vec3b>(cv::Point(x, y));
            blockBrightness += pixel[2];
        }
    }
    blockBrightness = blockBrightness / (image.blockHeight * image.blockWidth);
    image.blocks.push_back({blockBrightness});
}
void LSC::genValues(Image &image, cv::Mat &img)
{
    int count = 0;
    int pixelOnRow = 0;
    int blocksFilled = 0;
    for (int i = 0; i < blocksForHeight; i++)
    {
        for (int j = 0; j < blocksForWidth; j++)
        {

            genPixelValues(image, img, j, i);
        }
    }

    image.averageBrightness = getNormalizedvalues(image);
}

void LSC::saveValues(const Image &image)
{
    std::fstream writeFile;
    writeFile.open("../genValues.txt", std::ios::out);
    if (!writeFile)
    {
        printf("File not opened");
    }
    else
    {
        for (int i = 0; i < image.blocks.size(); i++)
        {
            float value = (1.0 / (float(image.blocks[i]) / image.averageBrightness));
            writeFile << value << std::endl;
        }
        writeFile.close();
    }
}
void LSC::loadValues(Image &image)
{
    float value;
    std::fstream readFile;
    readFile.open("../genValues.txt", std::ios::in);
    if (!readFile)
    {
        printf("File not opened");
    }
    else
    {
        std::string str = "";
        int count = 0;
        while (readFile >> str)
        {
            value = std::stof(str);
            image.blocks.push_back(value);
        }
    }
}
int clamp(int a, float b)
{
    if (a * b > 255.f)
    {
        return 255;
    }
    else if (a * b < 0.f)
    {
        return 0;
    }
    else
    {
        return int(a * b);
    }
}
void calculate(Image &image, cv::Mat &img, int &x, int &y, int j, int i, float &value1, float &value2, float &value3, float &value4)
{
    float horizontalValue1 = (float(image.blockWidth) * 0.5f + j) / float(image.blockWidth);
    float verticalValue1 = (float(image.blockHeight) * 0.5f + i) / float(image.blockHeight);
    float alpha1 = (horizontalValue1 * value2) + ((1.0f - horizontalValue1) * value1);
    float alpha2 = (horizontalValue1 * value4) + ((1.0f - horizontalValue1) * value3);
    float alphaFinal = (verticalValue1 * alpha2) + ((1.0f - verticalValue1) * alpha1);
    cv::Vec3b pix = img.at<cv::Vec3b>(cv::Point(x, y));
    // pix[0] = pix[1] = pix[2] = clamp(255, alphaFinal);
    pix[2] = clamp(pix[2], alphaFinal);
    img.at<cv::Vec3b>(cv::Point(x, y)) = pix;
}
void valuesForFirstSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image)
{
    if (posY == 0 && posX == 0)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock];
    }
    else if (posY == 0)
    {
        value1 = image.blocks[posBlock - 1];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock - 1];
        value4 = image.blocks[posBlock];
    }
    else if (posX == 0)
    {
        value1 = image.blocks[posBlock - blocksForWidth];
        value2 = image.blocks[posBlock - blocksForWidth];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock];
    }
}
void valuesForSecondSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image)
{
    value1 = image.blocks[posBlock - (blocksForWidth)];
    value2 = image.blocks[posBlock - (blocksForWidth - 1)];
    value3 = image.blocks[posBlock];
    value4 = image.blocks[posBlock + 1];
    if (posY == 0 && posX == blocksForWidth - 1)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock];
    }
    else if (posY == 0)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock + 1];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock + 1];
    }
    else if (posX == blocksForWidth - 1)
    {
        value1 = image.blocks[posBlock - blocksForWidth];
        value2 = image.blocks[posBlock - blocksForWidth];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock];
    }
}
void valuesForThirdSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image)
{
    value1 = image.blocks[posBlock - 1];
    value2 = image.blocks[posBlock];
    value3 = image.blocks[posBlock + (blocksForWidth - 1)];
    value4 = image.blocks[posBlock + blocksForWidth];
    if (posY == 0 && posX == 0)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock + blocksForWidth];
        value4 = image.blocks[posBlock + blocksForWidth];
    }
    else if (posX == 0 && posY == blocksForHeight - 1)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock];
    }
    else if (posX == 0)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock + blocksForWidth];
        value4 = image.blocks[posBlock + blocksForWidth];
    }

    else if (posY == blocksForHeight - 1)
    {
        value1 = image.blocks[posBlock - 1];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock - 1];
        value4 = image.blocks[posBlock];
    }
}
void valuesForFourthSubblock(float &value1, float &value2, float &value3, float &value4, const int &posX, const int &posY, const int &posBlock, const Image &image)
{
    value1 = image.blocks[posBlock];
    value2 = image.blocks[posBlock + 1];
    value3 = image.blocks[posBlock + blocksForWidth];
    value4 = image.blocks[posBlock + (blocksForWidth + 1)];

    if (posY == 0 && posX == blocksForWidth - 1)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock + blocksForWidth];
        value4 = image.blocks[posBlock + blocksForWidth];
    }
    else if (posX == blocksForWidth - 1 && posY == blocksForHeight - 1)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock];
    }
    else if (posY == blocksForHeight - 1)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock + 1];
        value3 = image.blocks[posBlock];
        value4 = image.blocks[posBlock + 1];
    }

    else if (posX == blocksForWidth - 1)
    {
        value1 = image.blocks[posBlock];
        value2 = image.blocks[posBlock];
        value3 = image.blocks[posBlock + blocksForWidth];
        value4 = image.blocks[posBlock + blocksForWidth];
    }
}
void applyPixelValues(Image &image, cv::Mat &img, const int &posX, const int &posY, const int &blockWidth, const int &blockHeight)
{
    int posBlock = posX + (posY * blocksForWidth);
    float value1 = image.blocks[posBlock - (blocksForWidth + 1)];
    float value2 = image.blocks[posBlock - blocksForWidth];
    float value3 = image.blocks[posBlock - 1];
    float value4 = image.blocks[posBlock];
    valuesForFirstSubblock(value1, value2, value3, value4, posX, posY, posBlock, image);

    for (int i = 0; i < blockHeight / 2; i++)
    {
        for (int j = 0; j < blockWidth / 2; j++)
        {
            int x = (j + (posX * image.blockWidth));
            int y = (i + (posY * image.blockHeight));

            calculate(image, img, x, y, j, i, value1, value2, value3, value4);
        }
    }
    valuesForSecondSubblock(value1, value2, value3, value4, posX, posY, posBlock, image);
    for (int i = 0; i < blockHeight / 2; i++)
    {
        for (int j = blockWidth / 2; j < blockWidth; j++)
        {
            int x = (j + (posX * image.blockWidth));
            int y = (i + (posY * image.blockHeight));

            calculate(image, img, x, y, (j - image.blockWidth), i, value1, value2, value3, value4);
        }
    }

    valuesForThirdSubblock(value1, value2, value3, value4, posX, posY, posBlock, image);
    for (int i = blockHeight / 2; i < blockHeight; i++)
    {
        for (int j = 0; j < blockWidth / 2; j++)
        {
            int x = (j + (posX * image.blockWidth));
            int y = (i + (posY * image.blockHeight));

            calculate(image, img, x, y, j, (i - image.blockHeight), value1, value2, value3, value4);
        }
    }
    valuesForFourthSubblock(value1, value2, value3, value4, posX, posY, posBlock, image);
    for (int i = blockHeight / 2; i < blockHeight; i++)
    {
        for (int j = blockWidth / 2; j < blockWidth; j++)
        {
            int x = (j + (posX * image.blockWidth));
            int y = (i + (posY * image.blockHeight));

            calculate(image, img, x, y, (j - image.blockWidth), (i - image.blockHeight), value1, value2, value3, value4);
        }
    }
}
void LSC::applyValues(Image &image, cv::Mat &img)
{
    for (int i = 0; i < blocksForHeight; i++)
    {
        for (int j = 0; j < blocksForWidth; j++)
        {
            int blockWidth = image.blockWidth;
            int blockHeight = image.blockHeight;
            if (j == blocksForWidth - 1)
            {
                blockWidth += image.requiredPixelsWidth;
            }
            if (i == blocksForHeight - 1)
            {
                blockHeight += image.requiredPixelsHeight;
            }
            applyPixelValues(image, img, j, i, blockWidth, blockHeight);
        }
    }

    cv::cvtColor(img, img, cv::COLOR_HSV2BGR);
    cv::imwrite("../img4.jpg", img);
}

void loadImage(Image &image)
{
    // vignette-effect-lighthouse

    // const char *filename = "../eitvae.jpeg";
    // char *filename = "../VIGN.jpg";
    char *filename = "../LSCOFF.jpg";
    stbi_info(filename, &image.width, &image.height, &image.channels);
    image.dataBuffer.resize(image.width * image.height * image.channels);
    unsigned char *imgData = stbi_load(filename, &image.width, &image.height, &image.channels, image.channels);
    memcpy(image.dataBuffer.data(), imgData, image.dataBuffer.size());
    stbi_image_free(imgData);
    fillImageData(image);
}
int main()
{

    Image image;
    loadImage(image);
    LSC lsc;
    cv::Mat img(image.height, image.width, CV_8UC4, image.input.data());
    cv::cvtColor(img, img, cv::COLOR_RGB2HSV);

    lsc.genValues(image, img);
    lsc.saveValues(image);
    image.blocks.clear();
    lsc.loadValues(image);
    lsc.applyValues(image, img);
    stbi_write_jpg("../saved.jpeg", image.width, image.height, 4, image.input.data(), 100);

    return 0;
}
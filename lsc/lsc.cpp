#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
const int blocksForWidth = 17;
const int blocksForHeight = 13;
struct Block
{
    float value;
    Block(float value) : value{value} {}
};
struct Pixel
{
    Pixel(unsigned char red, unsigned char green, unsigned char blue) : e{red, green, blue, 255} {}

    uint8_t e[4];
};
class Image
{
public:
    std::vector<Pixel> input;
    std::vector<Block> blocks;
    std::vector<uint8_t> dataBuffer;

    int width = 0;
    int height = 0;
    int channels = 0;

    int pixelMaxValue = 0;
    int blockWidth;
    int blockHeight;
    float averageBrightness;
};

struct LSC
{
    // move in Image
    void genValues(Image &image, cv::Mat &img);
    void saveValues(const Image &image);
    void loadValues(Image &image, Block &block);
    void applyValues(Image &image, cv::Mat &img);
};

void fillImageData(Image &image)
{
    size_t offset = 0;
    image.blockHeight = image.height / blocksForHeight;
    image.blockWidth = image.width / blocksForWidth;
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
    int sum = 0;
    for (int i = 0; i < image.blocks.size() - 1; i++)
    {
        sum += image.blocks[i].value;
    }
    return (sum / image.blocks.size());
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
        for (int i = 0; i <= image.blocks.size() - 1; i++)
        {
            float value = (1.0 / (float(image.blocks[i].value) / image.averageBrightness));
            writeFile << value << std::endl;
        }
        writeFile.close();
    }
}
void LSC::loadValues(Image &image, Block &block)
{
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
            block.value = std::stof(str);
            image.blocks.push_back(block);
        }
    }
}
int clamp(int a, float b)
{
    if (a * b > 255)
    {
        return 255;
    }
    else if (a * b < 0)
    {
        return 0;
    }
    else
    {
        return int(a * b);
    }
}
void applyPixelValues(Image &image, cv::Mat &img, int posX, int posY)
{
    for (int i = 0; i < image.blockHeight; i++)
    {
        for (int j = 0; j < image.blockWidth; j++)
        {
            int x = (j + (posX * image.blockWidth));
            int y = (i + (posY * image.blockHeight));
            cv::Vec3b pix = img.at<cv::Vec3b>(cv::Point(x, y));
            int posBlock = posX + (posY * blocksForWidth);
            int CenterBlock = (blocksForHeight / 2) * (blocksForWidth / 2);
            pix[2] = clamp(pix[2], image.blocks[posBlock].value);
            img.at<cv::Vec3b>(cv::Point(x, y)) = pix;
        }
    }
}
void LSC::applyValues(Image &image, cv::Mat &img)
{
    bool interpolate = false;
    for (int i = 0; i < blocksForHeight; i++)
    {
        for (int j = 0; j < blocksForWidth; j++)
        {

            applyPixelValues(image, img, j, i);
        }
    }

    cv::cvtColor(img, img, cv::COLOR_HSV2RGB);
    cv::imwrite("../img2.jpg", img);
}

void loadImage(Image &image)
{
    // vignette-effect-lighthouse
    stbi_info("../vignette-effect-lighthouse.jpg", &image.width, &image.height, &image.channels);
    image.dataBuffer.resize(image.width * image.height * image.channels);
    unsigned char *imgData = stbi_load("../vignette-effect-lighthouse.jpg", &image.width, &image.height, &image.channels, image.channels);
    memcpy(image.dataBuffer.data(), imgData, image.dataBuffer.size());
    stbi_image_free(imgData);
}

void interpolateImage(cv::Mat &img, Image &image)
{
    for (int i = 0; i < blocksForHeight * blocksForWidth; i++)
    {
        for (int posY = 0; posY < image.blockHeight; posY++)
        {
            for (int posX = 0; posX < image.blockWidth; posX++)
            {
                        }
        }
    }
}
int main()
{

    Image image;
    loadImage(image);
    fillImageData(image);
    cv::Mat img(image.height, image.width, CV_8UC4, image.input.data());
    cv::cvtColor(img, img, cv::COLOR_BGR2HSV);
    LSC lsc;
    Block block(0);
    lsc.genValues(image, img);
    lsc.saveValues(image);
    image.blocks.clear();
    lsc.loadValues(image, block);
    lsc.applyValues(image, img);
    stbi_write_jpg("../saved.jpeg", image.width, image.height, 4, image.input.data(), 100);

    return 0;
}
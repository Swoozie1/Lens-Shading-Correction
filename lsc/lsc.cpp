#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stbi_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stbi_image_write.h"
#include "stbi_image_resize.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
const int blocksForWidth = 5;
const int blocksForHeight = 5;
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
    int max = 0;
    for (int i = 1; i < image.blocks.size(); i++)
    {
        if (image.blocks[i].value > image.blocks[i - 1].value)
        {
            max = image.blocks[i].value;
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
    printf("%.2f %.2f /", (horizontalValue1), (verticalValue1));
    cv::Vec3b pix = img.at<cv::Vec3b>(cv::Point(x, y));
    // pix[0] = pix[1] = pix[2] = clamp(255, alphaFinal);
    pix[2] = clamp(pix[2], alphaFinal);
    img.at<cv::Vec3b>(cv::Point(x, y)) = pix;
}
void applyPixelValues(Image &image, cv::Mat &img, int posX, int posY)
{
    int posBlock = posX + (posY * blocksForWidth);
    float value1 = image.blocks[posBlock - (blocksForWidth + 1)].value;
    float value2 = image.blocks[posBlock - blocksForWidth].value;
    float value3 = image.blocks[posBlock - 1].value;
    float value4 = image.blocks[posBlock].value;
    printf("\n%.2f %.2f %.2f %.2f \n", value1, value2, value3, value4);
    for (int i = 0; i < image.blockHeight / 2; i++)
    {
        for (int j = 0; j < image.blockWidth / 2; j++)
        {
            int x = (j + (posX * image.blockWidth));
            int y = (i + (posY * image.blockHeight));

            calculate(image, img, x, y, j, i, value1, value2, value3, value4);
        }
    }
    value1 = image.blocks[posBlock - (blocksForWidth)].value;
    value2 = image.blocks[posBlock - (blocksForWidth - 1)].value;
    value3 = image.blocks[posBlock].value;
    value4 = image.blocks[posBlock + 1].value;
    printf("\n%.2f %.2f %.2f %.2f \n", value1, value2, value3, value4);
    for (int i = 0; i < image.blockHeight / 2; i++)
    {
        for (int j = 0; j < image.blockWidth / 2; j++)
        {
            int x = (j + image.blockWidth / 2 + (posX * image.blockWidth));
            int y = (i + (posY * image.blockHeight));

            calculate(image, img, x, y, (j - image.blockWidth / 2), i, value1, value2, value3, value4);
        }
    }
    value1 = image.blocks[posBlock - 1].value;
    value2 = image.blocks[posBlock].value;
    value3 = image.blocks[posBlock + (blocksForWidth - 1)].value;
    value4 = image.blocks[posBlock + blocksForWidth].value;
    printf("\n%.2f %.2f %.2f %.2f \n", value1, value2, value3, value4);
    for (int i = 0; i < image.blockHeight / 2; i++)
    {
        for (int j = 0; j < image.blockWidth / 2; j++)
        {
            int x = (j + (posX * image.blockWidth));
            int y = (i + image.blockHeight / 2 + (posY * image.blockHeight));

            calculate(image, img, x, y, j, (i - image.blockHeight / 2), value1, value2, value3, value4);
        }
    }
    value1 = image.blocks[posBlock].value;
    value2 = image.blocks[posBlock + 1].value;
    value3 = image.blocks[posBlock + blocksForWidth].value;
    value4 = image.blocks[posBlock + (blocksForWidth + 1)].value;
    printf("\n%.2f %.2f %.2f %.2f \n", value1, value2, value3, value4);
    for (int i = 0; i < image.blockHeight / 2; i++)
    {
        for (int j = 0; j < image.blockWidth / 2; j++)
        {
            int x = (j + image.blockWidth / 2 + (posX * image.blockWidth));
            int y = (i + image.blockHeight / 2 + (posY * image.blockHeight));

            calculate(image, img, x, y, (j - image.blockWidth / 2), (i - image.blockHeight / 2), value1, value2, value3, value4);
        }
    }
}
void LSC::applyValues(Image &image, cv::Mat &img)
{
    bool interpolate = false;
    for (int i = 1; i < blocksForHeight - 1; i++)
    {
        for (int j = 1; j < blocksForWidth - 1; j++)
        {

            applyPixelValues(image, img, j, i);
        }
    }

    cv::cvtColor(img, img, cv::COLOR_HSV2BGR);
    cv::imwrite("../img4.jpg", img);
}

void loadImage(Image &image)
{
    // vignette-effect-lighthouse

    const char *filename = "../eitvae.jpeg";
    // char *filename = "../30.jpg";
    // char *filename = "../LSCOFF.jpg";
    stbi_info(filename, &image.width, &image.height, &image.channels);
    image.dataBuffer.resize(image.width * image.height * image.channels);
    unsigned char *imgData = stbi_load(filename, &image.width, &image.height, &image.channels, image.channels);
    memcpy(image.dataBuffer.data(), imgData, image.dataBuffer.size());
    stbi_image_free(imgData);
}
int main()
{

    Image image;
    loadImage(image);
    fillImageData(image);
    cv::Mat img(image.height, image.width, CV_8UC4, image.input.data());
    cv::cvtColor(img, img, cv::COLOR_RGB2HSV);
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
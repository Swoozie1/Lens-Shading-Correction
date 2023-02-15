#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#include <vector>
#include <fstream>
#include <opencv4/opencv2/opencv.hpp>
const int blocksForWidth = 13;
const int blocksForHeight = 17;
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
    void genValues(Image &image);
    void saveValues(const Image &image);
    void loadValues(Image &image, Block &block);
    void applyValues(Image &image);
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

void LSC::genValues(Image &image)
{

    float blockBrightness = 0;
    for (int count = 0; count < blocksForHeight; count++)
    {
        int blockCount = 0;
        for (int i = 0; i < image.width; i++, blockCount++)
        {
            for (int j = 0; j <= image.blockHeight; j++)
            {
                int possition = (image.blockHeight * count * image.width) + (j * image.width) + i;
                blockBrightness += ((0.299 * image.input[possition].e[0]) + (0.587 * image.input[possition].e[1]) + (0.114 * image.input[possition].e[2]));
            }
            if (blockCount == image.blockWidth)
            {
                blockBrightness = blockBrightness / (image.blockHeight * image.blockWidth);
                image.blocks.push_back({blockBrightness});
                blockCount = 0;
            }
        }
    }
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
void LSC::applyValues(Image &image)
{
    int count = 0;
    int pixelOnRow = 0;
    int blocksFilled = 0;
    int centerBlock = (blocksForHeight / 2) * (blocksForWidth / 2);
    for (int i = 0; i < image.width * image.height; pixelOnRow++)
    {

        int possition = (pixelOnRow + (count * image.width) - image.blockWidth);
        if (blocksFilled == 0 && count == 0)
        {
            possition = (pixelOnRow + (count * image.width));
        }
        if (count == 0 && blocksFilled != 0)
        {
            possition = (i + (count * image.width) - image.blockWidth);
            i++;
        }
        else if (pixelOnRow == image.blockWidth)
        {
            count++;
            pixelOnRow = 0;
        }
        else if (count == image.blockHeight)
        {
            count = 0;
            i += image.blockWidth;
            blocksFilled++;
        }
        image.input[possition].e[0] = image.input[possition].e[0] * image.blocks[blocksFilled].value;
        image.input[possition].e[1] = image.input[possition].e[1] * image.blocks[blocksFilled].value;
        image.input[possition].e[2] = image.input[possition].e[2] * image.blocks[blocksFilled].value;
    }
}
void loadImage(Image &image)
{
    stbi_info("../noLSC.jpg", &image.width, &image.height, &image.channels);
    image.dataBuffer.resize(image.width * image.height * image.channels);
    unsigned char *imgData = stbi_load("../noLSC.jpg", &image.width, &image.height, &image.channels, image.channels);
    memcpy(image.dataBuffer.data(), imgData, image.dataBuffer.size());
    stbi_image_free(imgData);
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
int main()
{

    Image image;
    loadImage(image);
    fillImageData(image);
    LSC lsc;
    Block block(0);
    lsc.genValues(image);
    image.averageBrightness = getNormalizedvalues(image);
    lsc.saveValues(image);
    image.blocks.clear();
    lsc.loadValues(image, block);
    lsc.applyValues(image);
    stbi_write_jpg("../saved.jpeg", image.width, image.height, 4, image.input.data(), 100);

    return 0;
}
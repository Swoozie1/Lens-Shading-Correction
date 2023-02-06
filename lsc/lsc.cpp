#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#include <vector>
#include <fstream>
const int blocksForWidth = 13;
const int blocksForHeight = 17;
struct Block
{
    Block(float rValue, float grValue, float gbValue, float bValue) : values{rValue, grValue, gbValue, bValue} {}

    float values[4];
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
    // std::string dataBuffer = "";

    int width = 0;
    int height = 0;
    int channels = 0;

    int pixelMaxValue = 0;
    int blockWidth;
    int blockHeight;
    // Image(int _width, int _height, int _channels)
    // {
    //     width = _width;
    //     height = _height;
    //     // channels = _channels;
    //     blockWidth = _width / blocksForWidth;
    //     blockHeight = _height / blocksForHeight;
    // }
};

struct LSC
{
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
    int newlineCounter = 0;
    for (int i = 0; i < blocksForHeight * blocksForWidth; i++)
    {
        float sumRed = 0.0;
        float sumGreen = 0.0;
        float sumBlue = 0.0;
        int pixelsPerRow = 0;
        for (int j = 0; j < image.blockWidth * image.blockHeight; j++, pixelsPerRow++)
        {
            int possition = 0;
            if (newlineCounter == 0)
            {
                possition = j + (i * image.blockWidth);
            }
            else
            {
                possition = (j + ((image.width * newlineCounter) - image.blockWidth));
            }
            if (pixelsPerRow == image.blockWidth && newlineCounter <= image.blockHeight)
            {
                newlineCounter++;
                pixelsPerRow = 0;
            }
            else
            {
                newlineCounter = 0;
            }

            sumRed += image.input[possition].e[0];
            sumGreen += image.input[possition].e[1];
            sumBlue += image.input[possition].e[2];
        }
        sumRed = sumRed / float(image.blockHeight * image.blockWidth);
        sumGreen = sumGreen / float(image.blockHeight * image.blockWidth);
        sumBlue = sumBlue / float(image.blockHeight * image.blockWidth);
        image.blocks.push_back({(sumRed / 100), (sumGreen / sumRed), (sumGreen / sumBlue), (sumBlue / 100)});
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
            writeFile << image.blocks[i].values[0] << " " << image.blocks[i].values[1] << " " << image.blocks[i].values[2] << " " << image.blocks[i].values[3] << std::endl;
        }
    }
    writeFile.close();
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
            block.values[count] = std::stof(str);
            count++;
            if (count == 4)
            {
                image.blocks.push_back(block);
                count = 0;
            }
        }
    }
}
void LSC::applyValues(Image &image)
{
    int count = 0;
    int possition = 0;
    int pixelOnRow = 0;
    int blocksFilled = 0;
    for (int i = 0; i <= image.width * image.height; pixelOnRow++)
    {
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
        // else if (blocksFilled == blocksForWidth)
        // {
        //     i += image.blockHeight * image.width;
        // }
        image.input[possition].e[0] = image.input[possition].e[0] * image.blocks[blocksFilled].values[1];
        image.input[possition].e[1] = image.input[possition].e[0] * image.blocks[blocksFilled].values[3];
        image.input[possition].e[2] = image.input[possition].e[0] * image.blocks[blocksFilled].values[1];
    }
}
int main()
{

    Image image;
    stbi_info("../minon.jpeg", &image.width, &image.height, &image.channels);
    image.dataBuffer.resize(image.width * image.height * image.channels);
    unsigned char *imgData = stbi_load("../minon.jpeg", &image.width, &image.height, &image.channels, image.channels);
    memcpy(image.dataBuffer.data(), imgData, image.dataBuffer.size());
    stbi_image_free(imgData);
    fillImageData(image);
    LSC lsc;
    Block block(0, 0, 0, 0);
    lsc.genValues(image);
    lsc.saveValues(image);
    image.blocks.clear();
    lsc.loadValues(image, block);
    lsc.applyValues(image);
    stbi_write_jpg("../saved2.jpeg", image.width, image.height, 4, image.input.data(), 100);

    return 0;
}
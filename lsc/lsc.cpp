#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#include <vector>
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
    void saveValues();
    void loadValues();
    void applyValues();
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
    // for (int i = 0; i < image.width; i++)
    {
        r = image.dataBuffer[offset + 0];
        g = image.dataBuffer[offset + 1];
        b = image.dataBuffer[offset + 2];
        // sscanf((char *)&image.dataBuffer[offset], "%c %c %c%n", &r, &g, &b, &megaOffset);
        image.input.push_back({r, g, b});
        offset += 3;
    }
} // FILE *file,

void LSC::genValues(Image &image)
{

    for (int i = 0; i < blocksForHeight * blocksForWidth; i++)
    {
        float sumRed = 0.0;
        float sumGreen = 0.0;
        float sumBlue = 0.0;
        int count = 0;
        for (int j = 0; j < image.blockWidth * image.blockHeight; j++, count++)
        {

            if (count == image.blockWidth)
            {
                        }

            sumRed += image.input[j].e[0];
            sumGreen += image.input[j].e[1];
            sumBlue += image.input[j].e[2];
        }
        sumRed = sumRed / float(image.blockHeight * image.blockWidth);
        sumGreen = sumGreen / float(image.blockHeight * image.blockWidth);
        sumBlue = sumBlue / float(image.blockHeight * image.blockWidth);
        image.blocks.push_back({sumRed, (sumGreen / sumRed), (sumGreen / sumBlue), sumBlue});
    }
}
int main()
{
    FILE *file;
    file = fopen("../LSC.jpg", "rb");
    if (!file)
    {
        printf("Failed to open file!\n");
        return -1;
    }
    Image image;
    stbi_info("../LSC.jpg", &image.width, &image.height, &image.channels);
    image.dataBuffer.resize(image.width * image.height * image.channels);
    unsigned char *imgData = stbi_load("../LSC.jpg", &image.width, &image.height, &image.channels, image.channels);
    memcpy(image.dataBuffer.data(), imgData, image.dataBuffer.size());
    stbi_image_free(imgData);
    fillImageData(image);
    // stbi_write_jpg("saved2.jpg", image.width, image.height, 4, image.input.data(), 100);
    fclose(file);
    LSC lsc;
    lsc.genValues(image);
    // for (int i = 0; i < 1; i++)
    // {
    //     float sum = 0;
    //     for (int j = 0; j < image.blockWidth; j += 3)
    //     {
    //         sum += img[j];
    //     }
    //     sum = sum / float(image.blockHeight * image.blockWidth);
    //     printf("%f", sum);
    // }
    return 0;
}
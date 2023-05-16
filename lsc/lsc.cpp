#include "lsc.hpp"
#include "const.hpp"
#include "image.hpp"
#include <fstream>
void LSC::genBlockCoefficients(Image &image, cv::Mat &img)
{
    int count = 0;
    int pixelOnRow = 0;
    int blocksFilled = 0;
    for (int i = 0; i < blocksForHeight; i++)
    {
        for (int j = 0; j < blocksForWidth; j++)
        {
            image.genPixelValues(image, img, j, i);
        }
    }

    image.maxBrightness = image.getNormalizedvalues(image);
}
void LSC::saveBlockCoefficients(const Image &image)
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
            float value = (1.0 / (float(image.blocks[i]) / image.maxBrightness));
            writeFile << value << std::endl;
        }
        writeFile.close();
    }
}
void LSC::loadBlockCoefficients(Image &image)
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

void LSC::applyBlockCoefficients(Image &image, cv::Mat &img, const char *filename)
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
            image.applyPixelValues(image, img, j, i, blockWidth, blockHeight);
        }
    }

    cv::cvtColor(img, img, cv::COLOR_HSV2BGR);
    std::string lsced = "../LSCED";
    std::string finalname = lsced + &filename[3];
    std::cout << filename << " " << finalname;
    cv::imwrite(finalname, img);
}
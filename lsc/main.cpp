#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
#include "image.hpp"
#include "lsc.hpp"
#include "const.hpp"

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "createLSC") == 0)
    {
        for (int i = 2; i < argc; i++)
        {
            Image image;
            image.loadImage(image, argv[i]);
            LSC lsc;
            cv::Mat img(image.height, image.width, CV_8UC4, image.input.data());
            cv::cvtColor(img, img, cv::COLOR_RGB2HSV);
            lsc.genBlockCoefficients(image, img);
            lsc.saveBlockCoefficients(image);
            image.blocks.clear();
            lsc.loadBlockCoefficients(image);
            lsc.applyBlockCoefficients(image, img, argv[i]);
        }
    }
    else if (argc > 1 && strcmp(argv[1], "applyLSC") == 0)
    {
        FILE *file;
        if (file = fopen("../genValues.txt", "r"))
        {
            fclose(file);
            for (int i = 2; i < argc; i++)
            {
                Image image;
                image.loadImage(image, argv[i]);
                LSC lsc;
                cv::Mat img(image.height, image.width, CV_8UC4, image.input.data());
                cv::cvtColor(img, img, cv::COLOR_RGB2HSV);
                lsc.loadBlockCoefficients(image);
                lsc.applyBlockCoefficients(image, img, argv[i]);
            }
        }
        else
        {
            std::cout << "No correction to be applyed." << std::endl;
        }
    }
    return 0;
}
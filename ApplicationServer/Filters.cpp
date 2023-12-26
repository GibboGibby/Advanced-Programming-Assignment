#include "Filters.h"

cv::Mat Rotate::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    cv::Mat newImg;
    if (params[0] == "90")
    {
        cv::rotate(img, newImg, cv::ROTATE_90_CLOCKWISE);
    }
    else
        cv::rotate(img, newImg, cv::ROTATE_90_COUNTERCLOCKWISE);
    return newImg;
}
Rotate::Rotate()
{

}

Greyscale::Greyscale()
{

}

cv::Mat Greyscale::RunFilter(cv::Mat& img, std::vector<std::string>& params)
{
    cv::Mat newImg = img;
    for (int i = 0; i < newImg.rows; i++)
    {
        for (int j = 0; j < newImg.cols; j++)
        {
            cv::Vec3b pixel = newImg.at<cv::Vec3b>(i, j);

            //BGR
            //0.299 * red | 0.587 * green | 0.114 * blue
            int pixelVal = 0;
            pixelVal = pixel[0] * 0.114;
            pixelVal = pixel[1] * 0.587;
            pixelVal = pixel[2] * 0.299;
            pixel[0] = pixelVal;
            pixel[1] = pixelVal;
            pixel[2] = pixelVal;

            newImg.at<cv::Vec3b>(i, j) = pixel;
        }
    }
    return newImg;
}
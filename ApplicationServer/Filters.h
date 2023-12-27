#pragma once
#include "Filter.h"

class Rotate : public Filter
{
public:
	Rotate();
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);
};

class Greyscale : public Filter
{
public:
	Greyscale();
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);
};

class Flip : public Filter
{
public:
	Flip();
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);
};

class Resize : public Filter
{
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);

};

class Crop : public Filter
{
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);
};

class BoxBlur : public Filter
{
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);
};
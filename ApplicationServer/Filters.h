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
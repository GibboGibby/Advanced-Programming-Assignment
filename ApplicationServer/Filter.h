#pragma once
#include "Core.h"
#define NUM_THREADS 4
class Filter
{
public:
	virtual cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params) = 0;
	//void SetParams(std::vector<std::string>& params) { _params = params; }
	//void SetImage(cv::Mat& img) { _img = img; }
};
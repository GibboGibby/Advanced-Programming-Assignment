#pragma once
#include "Core.h"
#include "GibExceptions.h"
#define NUM_THREADS 4


class Filter
{
public:
	virtual cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params) throw(GibException*) = 0;
};
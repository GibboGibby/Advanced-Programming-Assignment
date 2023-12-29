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
private:
	std::mutex mutex;
public:
	Greyscale();
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);
	cv::Mat GrayscaleSingleThreaded(cv::Mat& img);
	cv::Mat GrayscaleMultiThreaded(cv::Mat& img);
	void GrayscaleThread(cv::Mat& origImg, cv::Mat tempImg, int startPos, int size);
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

class Sharpening : public Filter
{
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params);
};
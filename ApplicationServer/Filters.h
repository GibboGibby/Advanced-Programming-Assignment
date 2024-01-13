#pragma once
#include "Filter.h"

class Rotate : public Filter
{
public:
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class Greyscale : public Filter
{
private:
	std::mutex _mutex;
	cv::Mat GrayscaleSingleThreaded(cv::Mat& img);
	cv::Mat GrayscaleMultiThreaded(cv::Mat& img);
public:
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class Flip : public Filter
{
public:
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class Resize : public Filter
{
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);

};

class Crop : public Filter
{
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class BoxBlur : public Filter
{
private:
	std::mutex _mutex;
	cv::Mat BoxBlurSingleThreaded(cv::Mat& img, int sizeX, int sizeY);
	cv::Mat BoxBlurMutliThreaded(cv::Mat& img, int sizeX, int sizeY);
	void BoxBlurThread(cv::Mat& origImg, cv::Mat tempImg, int startPos, int size, int sizeX, int sizeY);
public:
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class Sharpening : public Filter
{
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class BrightnessAdjust : public Filter
{
private:
	std::mutex _mutex;
	cv::Mat BrightnessAdjustMultithreaded(cv::Mat& img, int brightness);
public:
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class GammaCorrection : public Filter
{
private:
	std::mutex _mutex;
public:
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};

class ContrastAdjust : public Filter
{
private:
	std::mutex _mutex;
public:
	cv::Mat RunFilter(cv::Mat& img, std::vector<std::string>& params)throw(GibException*);
};
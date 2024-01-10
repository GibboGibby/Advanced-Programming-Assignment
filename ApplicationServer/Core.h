#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <string>
#include <vector>
#include <map>
#include <iostream>

#define SERVER "127.0.0.1"
#define PORT 8888

#define UDP_BUF_SIZE 1357
#define HASH_ACCEPTABLE_ERROR 2.0

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include "opencv2/img_hash/average_hash.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "Filter.h"

#include <type_traits>


namespace GibCore
{
	

	inline double CalculateHash(cv::Mat& img)
	{
		cv::Mat hashedImg;
		cv::Ptr<cv::img_hash::AverageHash> m = cv::img_hash::AverageHash::create();
		m->compute(img, hashedImg);
		double avgSum = 0.0;
		double count = 0;
		for (int i = 0; i < hashedImg.cols; i++)
		{
			//std::cout << i << " - " << int(hashedImg.at<uchar>(0, i)) << std::endl;
			avgSum += double(hashedImg.at<uchar>(0, i));
			count++;
		}
		double avg = avgSum / count;
		std::cout << "This is the average - " << avg << std::endl;
		return avg;
	}

	enum class ImageFilter
	{
		RESIZE,
		ROTATION,
		CROPPING,
		FLIPPING,
		BRIGHTNESSADJUST,
		CONTRASTADJUST,
		GAMMACORRECTION,
		TOHSV,
		TOGREYSCALE,
		TORGB,
		GAUSSIANBLUR,
		BOXBLUR,
		SHARPENING,
		NOTHING
	};


	struct ImageFilterParams
	{
		ImageFilter filter;
		char params[1024];
		char ext[4];
	};

	/*
	std::pair<ImageFilter, int>(ImageFilter::RESIZE, 2),
	std::pair<ImageFilter, int>(ImageFilter::ROTATION, 1)
	};
	std::map<ImageFilter, int> imageFilterParams = {
	*/
	const std::map<ImageFilter, int> ImageFilterParamsSize = {
		{ImageFilter::RESIZE, 2},
		{ImageFilter::ROTATION, 1},
		{ImageFilter::TOGREYSCALE, 0},
		{ImageFilter::FLIPPING, 1},
		{ImageFilter::CROPPING, 2},
		{ImageFilter::BOXBLUR, 0},
		{ImageFilter::SHARPENING, 0},
		{ImageFilter::BRIGHTNESSADJUST, 1},
		{ImageFilter::GAMMACORRECTION, 1},
		{ImageFilter::CONTRASTADJUST, 1}
	};
	//extern std::map<ImageFilter, >

	//Factory
	
	inline cv::Mat FilterLambda(cv::Mat& img, const std::function <void(cv::Mat& img, int x, int y)>& f)
	{
		cv::Mat newImg = img.clone();
		for (int i = 0; i < newImg.rows; i++)
		{
			for (int j = 0; j < newImg.cols; j++)
			{
				f(newImg, i, j);
			}
		}
		return newImg;
	}

	inline cv::Mat FilterLambdaParallel(std::mutex& mutex, cv::Mat& img, int startPos, int size, const std::function<void(cv::Mat& img, int x, int y)>& f)
	{
		cv::Mat newImg = img.clone();
		for (int i = startPos; i < startPos + size; i++)
		{
			if (i > newImg.cols) continue;
			for (int j = 0; j < newImg.cols; j++)
			{
				f(newImg, i, j);
			}
		}
		mutex.lock();
		for (int i = startPos; i < startPos + size; i++)
		{
			for (int j = 0; j < newImg.cols; j++)
			{
				img.at<cv::Vec3b>(i, j) = newImg.at<cv::Vec3b>(i, j);
			}
		}
		mutex.unlock();

		return img;
	}

	inline cv::Mat MultithreadedImageProcessing(std::mutex& mutex, cv::Mat& img, int threadCount, const std::function<void(cv::Mat& img, int x, int y)>& f)
	{
		cv::Mat newImg = img.clone();
		int size = std::round((double)img.rows / threadCount);
		std::vector<std::thread> threads;
		for (int i = 0; i < threadCount; i++)
		{
			std::thread t1(&GibCore::FilterLambdaParallel, std::ref(mutex), std::ref(newImg), i * size, size, f);
			threads.emplace_back(std::move(t1));
		}
		for (int i = 0; i < threadCount; i++)
		{
			threads[i].join();
		}
		return newImg;
	}

	inline uchar Clamp2(int pixelCol, int minVal, int maxVal)
	{
		if (pixelCol > maxVal)
		{
			return (uchar)maxVal;
		}
		else if (pixelCol < minVal)
		{
			return (uchar)minVal;
		}
		else
		{
			return (uchar)pixelCol;
		}
	}
	
	inline bool DoubleCloseEnough(double val1, double val2, double error)
	{
		if (val1 < val2 + error && val1 > val2 - error) return true;
		else return false;
	}
}





class Filter;
template<typename T>
T* CreateFilter()
{
	static_assert(std::is_base_of<Filter, T>::value, "T must inherit from the Filter class");
	return new T();
}
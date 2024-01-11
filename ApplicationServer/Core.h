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
#define UDP_TIMEOUT_MS = 10000

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include "opencv2/img_hash/average_hash.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "Filter.h"

#include <type_traits>


namespace GibCore
{
	
	/// <summary>
	/// Calculates the average hash of a given image
	/// </summary>
	/// <param name="img">Image</param>
	/// <returns>Hash of the image</returns>
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

	// Enum of ImageFilter
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

	// Struct containing the arguments and the enum stating what filter to use
	struct ImageFilterParams
	{
		ImageFilter filter;
		char params[1024];
	};

	// Map of the enum and the amount of required arguements
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

	//Factory
	
	/// <summary>
	/// Filter an image on a single thread using a lambda
	/// </summary>
	/// <param name="img">image</param>
	/// <param name="f">function to perform</param>
	/// <returns>Filtered image</returns>
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

	/// <summary>
	/// Filter lambda for use on a thread
	/// </summary>
	/// <param name="mutex">mutex of class</param>
	/// <param name="img">image</param>
	/// <param name="startPos">Start position for the threaded chunk</param>
	/// <param name="size">Size of threaded chunk</param>
	/// <param name="f">Image function to perform</param>
	/// <returns>Filtered Image</returns>
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
	/// <summary>
	/// Creates a set of threads based on the thread count to run a lambda function on every pixel in a given range of rows
	/// </summary>
	/// <param name="mutex">mutex of the class so that we can lock the thread for the copying procedure</param>
	/// <param name="img">Reference to the image</param>
	/// <param name="threadCount">The number of threads to use</param>
	/// <param name="f">The function to perform on every element</param>
	/// <returns>Returns image</returns>
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

	/// <summary>
	/// Clamps uchar between two values
	/// </summary>
	/// <param name="pixelCol">colour of the pixel</param>
	/// <param name="minVal">the minimum value</param>
	/// <param name="maxVal">the maximum value</param>
	/// <returns>clamped value</returns>
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
	/// <summary>
	/// Checks to see if two doubles are within a certain range of eachother
	/// </summary>
	/// <param name="val1">First Value</param>
	/// <param name="val2">Second Value</param>
	/// <param name="error">Error Margin</param>
	/// <returns>Whether the values are close within the error </returns>
	inline bool DoubleCloseEnough(double val1, double val2, double error)
	{
		if (val1 < val2 + error && val1 > val2 - error) return true;
		else return false;
	}
}




/// <summary>
/// Factory setup that creates a filter object of whatever type is supplied as T
/// </summary>
class Filter;
template<typename T>
T* CreateFilter()
{
	// Assert makes sure that the type defined is a child of Filter
	static_assert(std::is_base_of<Filter, T>::value, "T must inherit from the Filter class");
	return new T();
}
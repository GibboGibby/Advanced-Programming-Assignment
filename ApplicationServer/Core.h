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

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include "opencv2/img_hash/average_hash.hpp"


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
		CHANGECOLORSPACE,
		GAUSSIANBLUR,
		BOXBLUR,
		SHARPENING,
		NOTHING
	};


	struct ImageFilterParams
	{
		ImageFilter filter;
		char params[1024];
	};

	//extern std::map<ImageFilter, >
	
}
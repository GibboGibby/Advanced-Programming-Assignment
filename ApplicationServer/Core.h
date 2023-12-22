#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <string>
#include <vector>
#include <map>
#include <iostream>

#define SERVER "127.0.0.1"
#define PORT 8889

#define UDP_BUF_SIZE 1357

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"


namespace GibCore
{

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
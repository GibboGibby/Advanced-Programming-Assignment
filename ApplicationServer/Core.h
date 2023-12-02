#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <string>
#include <vector>

#define SERVER "127.0.0.1"
#define PORT 8888

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"


namespace GibCore
{

	class InsideClass
	{
	private:
		int val = 52;

	public:
		void SetVal(int num) { val = num; }
		int GetVal() { return val; }
	};

	struct InsideSentStruct
	{
		int x, y;
	};

	struct SentStruct
	{
		uchar* imgSizeAsUCharPtr;
		size_t imgSize;
		std::string name;
		int id;
		InsideSentStruct vec;
		InsideClass theClass;
	};

	struct Image
	{
		size_t imgSize;
		char* imgData;
	};



}
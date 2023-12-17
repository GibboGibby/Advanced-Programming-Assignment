#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "..\ApplicationServer\Core.h"
#include <iostream>

#include <chrono>
#include <thread>
#include "UDPClient.h"

void PrintHelp()
{
	std::cout << "Help stuff" << std::endl;
}



int main(int argc, char* argv[])
{
	std::vector<std::string> args = {};
	for (int i = 1; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	if (args.size() == 0)
	{
		PrintHelp();
		return 1;
	}
	if (args[0] == "help")
	{
		PrintHelp();
		return 1;
	}

	// args contains the input parameters

	// Create client object

	UDPClient client;

	std::string delim = ":";
	std::string left = args[0].substr(0, args[0].find(delim));
	std::string right = args[0].substr(args[0].find(delim)+1, args[0].length());

	client.Init(std::stoi(right), left);
	client.CreateSocket();

	cv::Mat img;
	std::string extension;

	bool canLoad = client.LoadImageFromPath(args[1], img, extension);
	if (!canLoad)
	{
		std::cout << "Failed to load image" << std::endl;
		return 1; 
	}

	//cv::imshow("Gaming", img);
	//cv::waitKey(0);
	//cv::destroyWindow("Gaming");

	client.SendImage(img, extension);
	GibCore::ImageFilterParams param;
	param.filter = client.FilterFromString(args[2]);
	std::string longStr;
	for (int i = 3; i < args.size(); i++)
	{
		longStr += args[i] + " ";
	}

	strcpy_s(param.params, longStr.c_str());
	//std::copy(args.begin() + 3, args.end(), param.params);
	client.SendFilter(param);


	client.CloseAndCleanup();
}
//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "..\ApplicationServer\Core.h"
#include <iostream>

#include <chrono>
#include <thread>
#include "UDPClient.h"

// Print out help information
void PrintHelp()
{
	std::cout << "usage: <ip>:<port> <filter> <filter parameters> ..." << std::endl;
	std::cout << "Fitlers: " << std::endl;

	std::cout << "			- resize <x> <y> [new size]" << std::endl;
	std::cout << "			- rotation <degrees> [must be in 90 degree increments]" << std::endl;
	std::cout << "			- grayscale" << std::endl;
	std::cout << "			- flip <side> [horizontal, vertical or both]" << std::endl;
	std::cout << "			- crop <x> <y> [crops from 0,0 to x,y]" << std::endl;
	std::cout << "			- crop <x1> <y1> <x2> <y2> [crops from x1, y1 to x2, y2]" << std::endl;
	std::cout << "			- boxblur <x size> <y size> <[optional] parallel> [size of the box blur block]" << std::endl;
	std::cout << "			- sharpen" << std::endl;
	std::cout << "			- brightness <brightness> [between -255 and 255]" << std::endl;
	std::cout << "			- gamma <gamma> [gamma correction value]" << std::endl;
	std::cout << "			- contrast <contrast> [between -255 and 255]" << std::endl;
}


int main(int argc, char* argv[])
{
	// Load args into vector
	std::vector<std::string> args = {};
	for (int i = 1; i < argc; i++)
	{
		args.push_back(argv[i]);
	}

	if (args.size() == 0)
	{
		PrintHelp();
		return -1;
	}
	if (args[0] == "help" || argc < 3)
	{
		PrintHelp();
		return -1;
	}


	// args contains the input parameters

	// Create client object
	UDPClient client;

	// Split argument into server ip and port
	std::string delim = ":";
	std::string left = args[0].substr(0, args[0].find(delim));
	std::string right = args[0].substr(args[0].find(delim) + 1, args[0].length());

	// Read the image filter params from the arguments
	GibCore::ImageFilterParams param;
	param.filter = client.FilterFromString(args[2]);
	if (param.filter == GibCore::ImageFilter::NOTHING)
	{
		std::cout << "Not a valid filtering option" << std::endl;
		return -1;
	}

	// Checks to see if the correct number of arguments have been passed for the specific image filter (Too many is fine, too few bad)
	int amount = args.size() - 3;
	int val = GibCore::ImageFilterParamsSize.find(param.filter)->second;
	if (amount < val)
	{
		std::cout << "incorrect parameters typed! - " << args[2] << " requres " << val << " parameters" << std::endl;
		PrintHelp();
		return -1;
	}

	int charAmount = 0;
	for (int i = 3; i < args.size(); i++)
	{
		charAmount += args[i].length();
	}
	if (charAmount > 1024)
	{
		std::cout << "Too many parameter characters have been typed - the limit is 1024 characters" << std::endl;
		PrintHelp();
		return -1;
	}

	// Initialize Winsock
	try
	{
		if (!client.Init(std::stoi(right), left))
		{
			WSACleanup();
			return 1;
		}
	}
	catch (std::exception e)
	{
		std::cout << "Invalid port value has been set, please try again" << std::endl;
	}
	
	// Create socket
	if (!client.CreateSocket())
	{
		WSACleanup();
		return 1;
	}
	// Load image from the path from the args into the img object
	// Extension stores the last 4 elements from the path string
	cv::Mat img;
	std::string extension;
	bool canLoad = client.LoadImageFromPath(args[1], img, extension);
	if (!canLoad)
	{
		std::cout << "Failed to load image" << std::endl;
		return 1; 
	}



	// Send image object using udp
	std::cout << "Sending image!" << std::endl;

	//client.SendImageMultiThreaded(img, extension, img2, extension2);
	client.SendImage(img, extension);
	


	
	


	std::cout << "Both threads executed" << std::endl;
	// Create and send filter object
	// Contains what filter to use and the parameters
	// Converts remaining parameters into a long string
	std::string longStr;
	for (int i = 3; i < args.size(); i++)
	{
		longStr += args[i] + " ";
	}
	// Copy string into char[]
	strcpy_s(param.params, longStr.c_str());

	// Send the filter to the server
	client.SendFilter(param);

	bool success = client.CheckImageSuccessfullyFiltered(client.GetSocket());
	if (!success) return -1;

	cv::Mat recvimg = client.ReceieveImage(client.GetSocket());
	std::string path;
	int ext = strlen(args[1].c_str());
	for (int i = 0; i < ext -4; i++)
	{
		path += args[1][i];
	}
	path += "_" + args[2];// + extension;

	for (int i = 3; i < args.size(); i++)
	{
		path += "_" + args[i];
	}
	path += extension;
	
	std::cout << "new path";
	client.SaveImage(recvimg, path);

	client.CloseAndCleanup();
}
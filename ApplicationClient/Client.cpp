#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "..\ApplicationServer\Core.h"
#include <iostream>

#include <chrono>
#include <thread>
#include "UDPClient.h"

// Print out help information
void PrintHelp()
{
	std::cout << "Help stuff" << std::endl;
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
		return 1;
	}
	if (args[0] == "help" || argc < 3)
	{
		PrintHelp();
		return 1;
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
		return 1;
	}

	// Checks to see if the correct number of arguments have been passed for the specific image filter (Too many is fine, too few bad)
	int amount = args.size() - 3;
	int val = GibCore::ImageFilterParamsSize.find(param.filter)->second;
	if (amount < val)
	{
		std::cout << "incorrect parameters typed! - " << args[2] << " requres " << val << " parameters" << std::endl;
		PrintHelp();
		return 1;
	}

	// Initialize Winsock
	if (!client.Init(std::stoi(right), left))
	{
		WSACleanup();
		return 1;
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

	//client.SendImage(img, extension);

	cv::Mat img2;
	std::string extension2;
	//std::string newStr;
	//newStr = args[1].substr(0, 2);
	//newStr += "smol.png";
	/*
* 
	if (client.LoadImageFromPath(newStr, img2, extension2))
	{
		std::cout << "failed to load second img\n";
		return 0;
	}
	*/
	//img2 = cv::imread(".\\smol.png");
	extension2 = ".png";

	//client.SendImageMultiThreaded(img, extension, img2, extension2);
	client.SendImage(img, extension);
	


	//std::thread si1(&SendImageToServer, std::ref(client), img, extension);
	/*
	std::thread si1(&SendImageToServer, std::ref(client), std::ref(img), std::ref(extension));
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	std::thread si2(&SendImageToServer, std::ref(client), std::ref(img2), std::ref(extension2));
	si1.join();
	si2.join();
	*/


	
	


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
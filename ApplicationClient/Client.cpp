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

void SendImageToServer(UDPClient cli, cv::Mat img, std::string ext)
{
	cli.SendImage(img, ext);
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
	if (args[0] == "help")
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

	
	std::thread si1(&SendImageToServer, std::ref(client), img, extension);
	//std::this_thread::sleep_for(std::chrono::milliseconds(20));
	std::thread si2(&SendImageToServer, std::ref(client), img, extension);
	si1.join();
	si2.join();
	
	


	std::cout << "Both threads executed" << std::endl;
	// Create and send filter object
	// Contains what filter to use and the parameters
	GibCore::ImageFilterParams param;
	param.filter = client.FilterFromString(args[2]);
	// Converts remaining parameters into a long string
	std::string longStr;
	for (int i = 3; i < args.size(); i++)
	{
		longStr += args[i] + " ";
	}
	// Copy string into char[]
	strcpy_s(param.params, longStr.c_str());

	// Send the filter to the server
	//client.SendFilter(param);


	client.CloseAndCleanup();
}
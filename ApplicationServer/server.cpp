#include <iostream>
#include "UDPServer.h"

int main()
{


	std::cout << "Hello from server" << std::endl;
	// Create server object
	UDPServer server;
	// Init server
	if (!server.Init())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}
	// Create Socket
	if (!server.CreateSocket())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}
	// Bind Server
	if (!server.BindServer())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		server.CloseAndCleanup();
		return 1;
	}
	
	while (true)
	{
		// Recevive and store image
		//cv::Mat image = server.ReceiveImage();
		server.ReceiveImageParallel();
		//Recieve and store filter
		/*
		GibCore::ImageFilterParams filterInfo = server.ReceiveFilter();

		//std::cout << "filter info filter - " << server.GetEnumFilterName(filterInfo.filter) << std::endl;

		//String stream to convert params into a vector of strings
		std::stringstream ss(filterInfo.params);
		std::vector<std::string> parameters;
		std::string temp;
		while (ss >> temp)
			parameters.push_back(temp);
		*/



		//cv::imshow("Display", image);
		//cv::waitKey(0);
		//cv::destroyWindow("Display");

		//Filters and stuff
		
		/*
		cv::Mat rotatedImg;
		cv::rotate(image, rotatedImg, cv::RotateFlags::ROTATE_90_CLOCKWISE);
		cv::imshow("Rotated img", rotatedImg);
		cv::waitKey(0);
		cv::destroyWindow("Rotated img");
		*/
	}

	return 0;
} 
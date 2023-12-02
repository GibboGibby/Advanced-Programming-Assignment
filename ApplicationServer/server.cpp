#include <iostream>
#include "UDPServer.h"

int main()
{
	/*
	cv::Mat img = cv::imread("C:/Users/james/Pictures/Screenshots/bad-yuumi.jpg");
	cv::namedWindow("ServerDisplay", cv::WINDOW_AUTOSIZE);
	cv::imshow("ServerDisplay", img);
	cv::waitKey(0);
	cv::destroyWindow("ServerDisplay");
	*/


	std::cout << "Hello from server" << std::endl;
	
	UDPServer server;
	if (!server.Init())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	return 0;
} 
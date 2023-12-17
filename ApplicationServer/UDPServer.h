#pragma once
#include "Core.h"
#include <iostream>
class UDPServer
{
private:
	SOCKET serverSocket;
	sockaddr_in server, client;
	int slen = sizeof(sockaddr_in);
public:
	bool Init();
	bool CreateSocket();
	bool BindServer();
	void StartReceiving();
	cv::Mat ReceiveImage();
	GibCore::ImageFilterParams RecieveFilter();

	std::string GetEnumFilterName(GibCore::ImageFilter filter);

	bool VerifyImage();

	cv::Mat Filter();

	void CloseAndCleanup();
};


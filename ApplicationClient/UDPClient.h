#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "..\ApplicationServer\Core.h"
#include <fstream>


class UDPClient
{
private:
	SOCKET clientSocket;
	sockaddr_in server;
	WSAData wsaData;

public:
	bool Init(int port = 8888, std::string ip = "127.0.0.1");
	bool CreateSocket();
	bool LoadImageFromPath(std::string path, cv::Mat& img, std::string& extension);
	void SendImage(cv::Mat& img, std::string extension);
	void SendFilter(GibCore::ImageFilterParams params);
	bool VerifyImage();

	void CloseAndCleanup();

	GibCore::ImageFilter FilterFromString(std::string);

};
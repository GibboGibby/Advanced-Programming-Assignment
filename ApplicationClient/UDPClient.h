#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "..\ApplicationServer\Core.h"
#include <fstream>
#include <mutex>


class UDPClient
{
private:
	SOCKET clientSocket;
	sockaddr_in server;
	WSAData wsaData;
	std::mutex mutex;

public:
	bool Init(int port = 8888, std::string ip = "127.0.0.1");
	bool CreateSocket();
	bool LoadImageFromPath(std::string path, cv::Mat& img, std::string& extension);
	void SendImage(cv::Mat& img, std::string extension);
	void SendImageMultiThreaded(cv::Mat& img, std::string extension, cv::Mat& img2, std::string extension2);
	void SendFilter(GibCore::ImageFilterParams params);
	bool VerifyImage();

	void CloseAndCleanup();

	GibCore::ImageFilter FilterFromString(std::string);

};
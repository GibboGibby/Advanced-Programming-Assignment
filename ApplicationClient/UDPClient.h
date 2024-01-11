#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "..\ApplicationServer\Core.h"
#include <fstream>
#include <mutex>


class UDPClient
{
private:
	SOCKET _clientSocket;
	sockaddr_in _server;
	WSAData _wsaData;
	std::mutex _mutex;

public:
	bool Init(int port = 8888, std::string ip = "127.0.0.1");
	bool CreateSocket();
	bool LoadImageFromPath(const std::string& path, cv::Mat& img, std::string& extension);
	
	void SendImage(cv::Mat& img, std::string extension);
	void SendImageMultiThreaded(cv::Mat& img, std::string extension, cv::Mat& img2, std::string extension2);

	GibCore::ImageFilter FilterFromString(std::string);
	void SendFilter(GibCore::ImageFilterParams params);
	bool VerifyImage(cv::Mat& img, SOCKET& clientSocket, sockaddr_in& tempServer);
	cv::Mat ReceieveImage(SOCKET& clientSocket);
	bool CheckImageSuccessfullyFiltered(SOCKET& socket);

	void SaveImage(cv::Mat& img, std::string name);

	SOCKET& GetSocket() { return _clientSocket; }

	void CloseAndCleanup();


private:

	void ExitProgram(std::string extra = "");

};
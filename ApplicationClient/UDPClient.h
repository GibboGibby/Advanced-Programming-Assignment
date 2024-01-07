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
	bool VerifyImage(cv::Mat& img, SOCKET& clientSocket, sockaddr_in& tempServer);
	cv::Mat ReceieveImage(SOCKET& clientSocket);

	void SaveImage(cv::Mat& img, std::string name);

	SOCKET& GetSocket() { return clientSocket; }

	void CloseAndCleanup();

	GibCore::ImageFilter FilterFromString(std::string);

private:
	void ExitProgram(std::string extra = "")
	{
		if (extra == "")
			std::cout << "A problem has occured program will now terminate" << std::endl;
		else
			std::cout << "Error: " << extra << " - Program will now Terminate" << std::endl;
		WSACleanup();
		closesocket(clientSocket);
		exit(EXIT_FAILURE);
	}
};
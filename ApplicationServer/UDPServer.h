#pragma once
#include "Core.h"
#include <iostream>
#include <mutex>
#include "Filters.h"

#define SERVER_THREADS 4

class UDPServer
{
private:
	SOCKET _serverSocket;
	sockaddr_in _server, _client;
	WSAData _wsaData;
	std::mutex _mutex;
	int _slen = sizeof(sockaddr_in);
	std::vector<std::thread::id> _threadIDs;
	std::vector<ULONG> _clients;
	std::vector<int> _usedPorts;
	
public:
	bool Init();
	bool CreateSocket();
	bool BindServer();
	void StartReceiving();
	cv::Mat ReceiveImage();
	void ReceiveImageParallel();
	void ReceivingAndProcessing(sockaddr_in client, size_t size, int port);
	GibCore::ImageFilterParams ReceiveFilter();

	

	std::string GetEnumFilterName(GibCore::ImageFilter filter);

	bool VerifyImage(cv::Mat& img, SOCKET& threadSocket, sockaddr_in client);

	GibCore::ImageFilterParams ReceieveFilter(SOCKET& threadSocket);
	cv::Mat FilterImage(cv::Mat& img, GibCore::ImageFilterParams& params);

	void SendImage(cv::Mat& img,std::string& ext, SOCKET& threadSocket, sockaddr_in clientSocket);

	//cv::Mat Filter();

	void CloseAndCleanup();

private:
	Filter* GetFilterFromEnum(GibCore::ImageFilter filter);

	void RemovePort(int port);
	void RemoveThread(std::thread::id id);

	void AddToClients(sockaddr_in client);
	bool CheckClients(sockaddr_in client);
	void RemoveFromClients(sockaddr_in client);

	void TerminateThread(SOCKET& socket, int& port);
	std::vector<std::string> GetVectorOfString(GibCore::ImageFilterParams);
};


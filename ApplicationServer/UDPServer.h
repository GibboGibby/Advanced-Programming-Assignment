#pragma once
#include "Core.h"
#include <iostream>
#include <mutex>
class UDPServer
{
private:
	SOCKET serverSocket;
	sockaddr_in server, client;
	WSAData wsaData;
	std::mutex mutex;
	int slen = sizeof(sockaddr_in);
	std::vector<std::thread::id> threadIDs;
	std::vector<ULONG> clients;
	std::vector<int> usedPorts;
	int randomVal = 0;
public:
	bool Init();
	bool CreateSocket();
	bool BindServer();
	void StartReceiving();
	cv::Mat ReceiveImage();
	void ReceiveImageParallel();
	void ReceivingAndProcessing(sockaddr_in client, size_t size, int port);
	GibCore::ImageFilterParams ReceiveFilter();

	void AddToClients(sockaddr_in client);
	bool CheckClients(sockaddr_in client);
	void RemoveFromClients(sockaddr_in client);

	std::string GetEnumFilterName(GibCore::ImageFilter filter);

	bool VerifyImage();

	cv::Mat Filter();

	void CloseAndCleanup();
};


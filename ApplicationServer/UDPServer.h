#pragma once
#include "Core.h"
#include <iostream>
#include <mutex>
#include "Filters.h"
#include "GibExceptions.h"

#define SERVER_THREADS 4

class UDPServer
{
private:
	int RECVTHREADS = SERVER_THREADS;
	int PROCESSTHREADS = SERVER_THREADS;
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
	void ReceiveImageParallel();
	void CloseAndCleanup();

private:

	void ReceivingAndProcessing(sockaddr_in client, size_t size, int port);

	Filter* GetFilterFromEnum(GibCore::ImageFilter filter);

	cv::Mat ReceiveImage(SOCKET& threadSocket, size_t size);

	GibCore::ImageFilterParams ReceiveFilter();

	std::string GetEnumFilterName(GibCore::ImageFilter filter);

	bool VerifyImage(cv::Mat& img, SOCKET& threadSocket, sockaddr_in client);

	GibCore::ImageFilterParams ReceieveFilter(SOCKET& threadSocket);
	cv::Mat FilterImage(cv::Mat& img, GibCore::ImageFilterParams& params) throw (GibException*);

	void SendImage(cv::Mat& img, std::string& ext, SOCKET& threadSocket, sockaddr_in clientSocket);

	void RemovePort(int port);

	void TerminateThread(SOCKET& socket, int& port);
	std::vector<std::string> GetVectorOfString(GibCore::ImageFilterParams);
};


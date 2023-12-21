#include "UDPServer.h"

bool UDPServer::Init()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}
	memset((char*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	return true;
}

	// https://gist.github.com/sunmeat/02b60c8a3eaef3b8a0fb3c249d8686fd
	// This is a great resource for learning.
	// Need to make the client a class and tidy this one up so not everything is done in the Init method
	// Struct sends good tho
	// Now need to test it with an opencv image

	// https://stackoverflow.com/questions/57794550/sending-large-files-over-udp
	// This is a great resource for learning how to send larger files across UDP
	// Seems quite simple just requires to split teh packet into a lot of smaller packets
	// I think recv from should let me receive all the packets all I need is to alter the send on the client side
	// As this is where the error is coming from

bool UDPServer::CreateSocket()
{
	if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		return false;
	}
	return true; 
}

bool UDPServer::BindServer()
{
	if (bind(serverSocket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

void UDPServer::StartReceiving()
{
	while (true)
	{
		cv::Mat image = ReceiveImage();


	}
}

void UDPServer::ReceiveImageParallel()
{
	sockaddr_in newClient;
	char sizeBuf[sizeof(size_t)];
	recvfrom(serverSocket, sizeBuf, sizeof(size_t), MSG_PEEK, (sockaddr*)&newClient, &slen);
	if (CheckClients(newClient)) {
		//std::cout << "Client already in the system\n";
		return;
	}
	//std::cout << "Sizebuf (before second recv) - " << sizeBuf << std::endl;
	recvfrom(serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&newClient, &slen);
	//std::cout << "Sizebuf (after second recv) - " << sizeBuf << std::endl;
	AddToClients(newClient);
	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));

	std::cout << "Size just after receive - " << actualSize << std::endl;
	std::thread thr(&UDPServer::ReceivingAndProcessing, this, newClient, actualSize);
	threadIDs.push_back(thr.get_id());
	thr.detach();
}

void UDPServer::ReceivingAndProcessing(sockaddr_in client, size_t size)
{

	//std::cout << GetCurrentThreadId() << " - " << client.sin_addr.S_un.S_addr << std::endl;
	std::cout << "\nCreating new thread\n";
	SOCKET threadSocket;
	threadSocket = socket(AF_INET, SOCK_DGRAM, 0);
	connect(threadSocket, (sockaddr*)&client, sizeof(sockaddr_in));
	char sizeBuf[sizeof(size_t)];
	//recvfrom(serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&client, &slen);


	char* buffer = new char[size];
	std::cout << "Parallel actual size - " << size << std::endl;
	size_t remainingToReceieve = size;
	
	char* bufferPos = &buffer[0];
	while (remainingToReceieve > 0)
	{
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		
		while (recvfrom(serverSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
		{
			printf("RecvFrom Failed!\nThis reason: %i\n", WSAGetLastError());
			printf("But this is being retried\n");
		}
		remainingToReceieve -= sendSize;
		bufferPos += sendSize;
		std::cout << "This is the amount received - " << remainingToReceieve << std::endl;
		
	}
	cv::Mat image;
	std::vector<uchar> imgData;
	imgData.assign(buffer, buffer + size);
	image = cv::imdecode(cv::Mat(imgData), 1);

	cv::imshow("Thread img", image);
	cv::waitKey(0);
	cv::destroyWindow("Thread img");

	mutex.lock();
	RemoveFromClients(client);
	for (int i = 0; i < threadIDs.size(); i++)
	{
		if (threadIDs[i] == std::this_thread::get_id())
		{
			threadIDs.erase(threadIDs.begin() + i);
			break;
		}
	}
	mutex.unlock();
}
//https://stackoverflow.com/questions/54155900/udp-server-and-connected-sockets
// For connect which should filter the packets. need to test on dans pc
/*
cv::Mat UDPServer::ReceiveImage()
{
	char sizeBuf[sizeof(size_t)];

	recvfrom(serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&client, &slen);
	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));
	std::cout << actualSize << " - Is the size transmitted across the wire" << std::endl;
	char* buffer = new char[actualSize];


	size_t remainingToReceieve = actualSize;

	char* bufferPos = &buffer[0];
	//if (len = recvfrom(serverSocket, buffer, sizeof(GibCore::SentStruct), 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
	while (remainingToReceieve > 0)
	{
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		while (recvfrom(serverSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
		{
			printf("RecvFrom Failed!\nThis reason: %s\n", WSAGetLastError());
			printf("But this is being retried\n");
		}
		remainingToReceieve -= sendSize;
		bufferPos += sendSize;
		std::cout << "This is the amount received - " << remainingToReceieve << std::endl;
	}
	cv::Mat image;
	std::vector<uchar> vec;
	vec.assign(buffer, buffer + actualSize);

	image = cv::imdecode(cv::Mat(vec), 1);
	return image;
}
*/

cv::Mat UDPServer::ReceiveImage()
{
	char sizeBuf[sizeof(size_t)];

	recvfrom(serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&client, &slen);
	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));
	char* buffer = new char[actualSize];

	std::cout << actualSize << " - Is the size transmitted across the wire" << std::endl;
	
	size_t remainingToReceieve = actualSize;

	char* bufferPos = &buffer[0];
	//if (len = recvfrom(serverSocket, buffer, sizeof(GibCore::SentStruct), 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
	while (remainingToReceieve > 0)
	{
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		while (recvfrom(serverSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
		{
			printf("RecvFrom Failed!\nThis reason: %i\n", WSAGetLastError());
			printf("But this is being retried\n");
		}
		remainingToReceieve -= sendSize;
		bufferPos += sendSize;
		std::cout << "This is the amount received - " << remainingToReceieve << std::endl;
	}
	cv::Mat image;
	std::vector<uchar> vec;
	vec.assign(buffer, buffer + actualSize);

	image = cv::imdecode(cv::Mat(vec), 1);
	return image;
}


GibCore::ImageFilterParams UDPServer::ReceiveFilter()
{
	GibCore::ImageFilterParams filterInfo;
	char imgfilterbuffer[sizeof(GibCore::ImageFilterParams)];
	recvfrom(serverSocket, imgfilterbuffer, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&client, &slen);
	memcpy(&filterInfo, imgfilterbuffer, sizeof(GibCore::ImageFilterParams));
	return filterInfo;
}

void UDPServer::AddToClients(sockaddr_in client)
{
	clients.push_back(client.sin_addr.s_addr);
}

bool UDPServer::CheckClients(sockaddr_in client)
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i] == client.sin_addr.s_addr)
			return true;
	}
	return false;
}

void UDPServer::RemoveFromClients(sockaddr_in client)
{
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i] == client.sin_addr.S_un.S_addr)
		{
			clients.erase(clients.begin() + i);
			return;
		}
	}
}

std::string UDPServer::GetEnumFilterName(GibCore::ImageFilter filter)
{
	switch (filter)
	{
	case GibCore::ImageFilter::RESIZE:
		return "Resize";
	case GibCore::ImageFilter::ROTATION:
		return "Rotation";
	case GibCore::ImageFilter::CROPPING:
		return "Cropping";
	case GibCore::ImageFilter::FLIPPING:
		return "Flipping";
	case GibCore::ImageFilter::BRIGHTNESSADJUST:
		return "BrightnessAdjust";
	case GibCore::ImageFilter::CONTRASTADJUST:
		return "ContrastAdjust";
	case GibCore::ImageFilter::GAMMACORRECTION:
		return "GammaCorrection";
	case GibCore::ImageFilter::CHANGECOLORSPACE:
		return "ChangeColorSpace";
	case GibCore::ImageFilter::GAUSSIANBLUR:
		return "GaussianBlur";
	case GibCore::ImageFilter::BOXBLUR:
		return "BoxBlur";
	case GibCore::ImageFilter::SHARPENING:
		return "Sharpening";
	default:
		return "Unknown";
	}
}

void UDPServer::CloseAndCleanup()
{
	closesocket(serverSocket);
	WSACleanup();
}

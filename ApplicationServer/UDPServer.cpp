#include "UDPServer.h"

bool UDPServer::Init()
{
	if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0)
	{
		return false;
	}
	memset((char*)&_server, 0, sizeof(_server));
	_server.sin_family = AF_INET;
	_server.sin_addr.s_addr = INADDR_ANY;
	_server.sin_port = htons(PORT);
	

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
	if ((_serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		return false;
	}
	int optval = 1;
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) & optval, sizeof(optval));
	return true; 
}

bool UDPServer::BindServer()
{
	if (bind(_serverSocket, (sockaddr*)&_server, sizeof(_server)) == SOCKET_ERROR)
	{
		return false;
	}
	_usedPorts.push_back(8888);
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

	recvfrom(_serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&newClient, &_slen);
	int confirmation = 1;
	if (_usedPorts.size() - 1 >= SERVER_THREADS)
	{
		confirmation = 0;
		std::cout << "Too many clients currently connecting. refusing connection";
	}
	sendto(_serverSocket, (char*)&confirmation, sizeof(int), 0, (sockaddr*)&newClient, _slen);
	if (confirmation == 0) return;
	AddToClients(newClient);
	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));
	int port = _usedPorts[_usedPorts.size() - 1] + 1;
	
	std::cout << "Now sending info back" << std::endl;

	_usedPorts.push_back(port);
	std::cout << "Size just after receive - " << actualSize << std::endl;
	std::thread thr(&UDPServer::ReceivingAndProcessing, this, newClient, actualSize, port);
	_threadIDs.push_back(thr.get_id());
	thr.detach();
}

void UDPServer::ReceivingAndProcessing(sockaddr_in client, size_t size, int port)
{
	//std::cout << GetCurrentThreadId() << " - " << client.sin_addr.S_un.S_addr << std::endl;
	std::cout << "\nCreating new thread on port - "<< port << std::endl;

	sendto(_serverSocket, (const char*)&port, sizeof(int), 0, (sockaddr*)&client, _slen);
	SOCKET threadSocket;
	threadSocket = socket(AF_INET, SOCK_DGRAM, 0);
	int optval = 1;
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	sockaddr_in threadServer;
	memset((char*)&threadServer, 0, sizeof(threadServer));
	threadServer.sin_family = AF_INET;
	threadServer.sin_addr.s_addr = _server.sin_addr.s_addr;
	threadServer.sin_port = htons(port);
	if (bind(threadSocket, (sockaddr*)&threadServer, sizeof(threadServer)) == SOCKET_ERROR)
	{
		std::cout << "Error binding another socket. error code - " << WSAGetLastError() << std::endl;
		TerminateThread(threadSocket, port);
		return;
	}
	std::cout << "Server bound successfully\n";
	//connect(threadSocket, (sockaddr*)&client, sizeof(sockaddr_in));
	char sizeBuf[sizeof(size_t)];
	//recvfrom(serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&client, &slen);


	char* buffer = new char[size];
	std::cout << "Parallel actual size - " << size << std::endl;
	size_t remainingToReceieve = size;
	
	char* bufferPos = &buffer[0];
	while (remainingToReceieve > 0)
	{
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		//sockaddr_in newClient;
		//std::cout << "Gets here\n";
		while (recvfrom(threadSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&client, &_slen) == SOCKET_ERROR)
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
	delete[] buffer;

	bool verified = VerifyImage(image, threadSocket, client);
	std::cout << "Verified value - " << verified << std::endl;
	if (verified == 0)
	{
		std::cout << "Image has not been recieved properly. Please try again" << std::endl;
		TerminateThread(threadSocket, port);
		return;
	}

	GibCore::ImageFilterParams params = ReceieveFilter(threadSocket);
	cv::Mat filteredImage = FilterImage(image, params);
	
	std::cout << "params ext - " << params.ext;
	std::string ext = ".png";
	SendImage(filteredImage, ext, threadSocket, client);

	
	//RemoveThread(std::this_thread::get_id());
	TerminateThread(threadSocket, port);
}
//https://stackoverflow.com/questions/54155900/udp-server-and-connected-sockets
// For connect which should filter the packets. need to test on dans pc

cv::Mat UDPServer::ReceiveImage()
{
	char sizeBuf[sizeof(size_t)];

	recvfrom(_serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&_client, &_slen);
	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));
	std::cout << actualSize << " - Is the size transmitted across the wire" << std::endl;
	char* buffer = new char[actualSize];

	size_t remainingToReceieve = actualSize;

	char* bufferPos = &buffer[0];
	while (remainingToReceieve > 0)
	{
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		while (recvfrom(_serverSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&_client, &_slen) == SOCKET_ERROR)
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

GibCore::ImageFilterParams UDPServer::ReceiveFilter()
{
	GibCore::ImageFilterParams filterInfo;
	char imgfilterbuffer[sizeof(GibCore::ImageFilterParams)];
	recvfrom(_serverSocket, imgfilterbuffer, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&_client, &_slen);
	memcpy(&filterInfo, imgfilterbuffer, sizeof(GibCore::ImageFilterParams));
	return filterInfo;
}

void UDPServer::AddToClients(sockaddr_in client)
{
	_clients.push_back(client.sin_addr.s_addr);
}

bool UDPServer::CheckClients(sockaddr_in client)
{
	for (int i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == client.sin_addr.s_addr)
			return true;
	}
	return false;
}

void UDPServer::RemoveFromClients(sockaddr_in client)
{
	for (int i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == client.sin_addr.S_un.S_addr)
		{
			_clients.erase(_clients.begin() + i);
			return;
		}
	}
}

void UDPServer::TerminateThread(SOCKET& socket, int& port)
{
	closesocket(socket);
	RemovePort(port);
	RemoveFromClients(_client);
}

std::vector<std::string> UDPServer::GetVectorOfString(GibCore::ImageFilterParams params)
{
	std::stringstream ss(params.params);
	std::vector<std::string> parameters;
	std::string temp;
	while (ss >> temp)
		parameters.push_back(temp);

	return parameters;
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
	case GibCore::ImageFilter::TOHSV:
		return "HSV";
	case GibCore::ImageFilter::TOGREYSCALE:
		return "Greyscale";
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

bool UDPServer::VerifyImage(cv::Mat& img, SOCKET& threadSocket, sockaddr_in client)
{
	double hash = GibCore::CalculateHash(img);
	char buf[sizeof(double)];
	sockaddr_in newClient;
	recvfrom(threadSocket, buf, sizeof(double), 0, (sockaddr*)&newClient, &_slen);
	double otherHash = 0;
	memcpy(&otherHash, buf, sizeof(double));
	bool isHash = GibCore::DoubleCloseEnough(hash, otherHash, HASH_ACCEPTABLE_ERROR);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	_mutex.lock();
	sendto(threadSocket, (char*) & isHash, sizeof(char), 0, (sockaddr*)&client, _slen);
	_mutex.unlock();
	return isHash;
}

GibCore::ImageFilterParams UDPServer::ReceieveFilter(SOCKET& threadSocket)
{
	GibCore::ImageFilterParams params;
	char paramsBuffer[sizeof(GibCore::ImageFilterParams)];
	// Receieve
	sockaddr_in newCli;
	recvfrom(threadSocket, (char*)paramsBuffer, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&newCli, &_slen);
	memcpy(&params, paramsBuffer, sizeof(GibCore::ImageFilterParams));
	return params;
}

cv::Mat UDPServer::FilterImage(cv::Mat& img, GibCore::ImageFilterParams& params)
{
	// Convert params.params into a vector of strings
	std::vector<std::string> parameters = GetVectorOfString(params);

	//Filter
	Filter* filter =  GetFilterFromEnum(params.filter);
	cv::Mat filteredImage = filter->RunFilter(img, parameters);
	return filteredImage;
}

void UDPServer::SendImage(cv::Mat& img, std::string& ext, SOCKET& threadSocket, sockaddr_in clientSocket)
{

	std::vector<uchar> buf;
	cv::imencode(ext, img, buf);
	size_t imgSize = buf.size();
	char sizeChar[sizeof(size_t)];
	memcpy(sizeChar, &imgSize, sizeof(size_t));
	while (sendto(threadSocket, sizeChar, sizeof(size_t), 0, (sockaddr*)&clientSocket, _slen) == SOCKET_ERROR)
	{
		std::cout << "Error sending the size back to the client" << std::endl;
	}

	size_t remainingToSend = buf.size();
	uchar* from = &buf[0];
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	while (remainingToSend > 0)
	{
		size_t sendSize = remainingToSend > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToSend;
		while (sendto(threadSocket, (const char*)from, sendSize, 0, (sockaddr*)&clientSocket, _slen) == SOCKET_ERROR)
		{
			printf("Error during send, retrying...\n");
		}
		remainingToSend -= sendSize;
		from += sendSize;
		std::cout << remainingToSend << " - What is left to send" << std::endl;
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}


	//sendto(threadSocket, (char*)&isHash, sizeof(char), 0, (sockaddr*)&client, slen);

	//while ()
	size_t remainingToReceieve;
}

void UDPServer::CloseAndCleanup()
{
	closesocket(_serverSocket);
	WSACleanup();
}

Filter* UDPServer::GetFilterFromEnum(GibCore::ImageFilter filter)
{
	//Filter* filter;

	switch (filter)
	{
	case GibCore::ImageFilter::ROTATION:
		return CreateFilter<Rotate>();
	case GibCore::ImageFilter::TOGREYSCALE:
		return CreateFilter<Greyscale>();
	case GibCore::ImageFilter::FLIPPING:
		return CreateFilter<Flip>();
	case GibCore::ImageFilter::RESIZE:
		return CreateFilter<Resize>();
	case GibCore::ImageFilter::CROPPING:
		return CreateFilter<Crop>();
	case GibCore::ImageFilter::BOXBLUR:
		return CreateFilter<BoxBlur>();
	case GibCore::ImageFilter::SHARPENING:
		return CreateFilter<Sharpening>();
	case GibCore::ImageFilter::BRIGHTNESSADJUST:
		return CreateFilter<BrightnessAdjust>();
	case GibCore::ImageFilter::GAMMACORRECTION:
		return CreateFilter<GammaCorrection>();
	case GibCore::ImageFilter::CONTRASTADJUST:
		return CreateFilter<ContrastAdjust>();
	}
}

void UDPServer::RemovePort(int port)
{
	_mutex.lock();
	for (int i = 0; i < _usedPorts.size(); i++)
	{
		if (_usedPorts[i] == port)
		{
			_usedPorts.erase(_usedPorts.begin() + i);
			break;
		}
	}
	_mutex.unlock();
}

void UDPServer::RemoveThread(std::thread::id id)
{
	_mutex.lock();
	for (int i = 0; i < _threadIDs.size(); i++)
	{
		if (_threadIDs[i] == id)
		{
			_threadIDs.erase(_threadIDs.begin() + i);
			break;
		}
	}
	_mutex.unlock();
}

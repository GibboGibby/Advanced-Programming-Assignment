#include "UDPServer.h"

bool UDPServer::Init()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);
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

GibCore::ImageFilterParams UDPServer::RecieveFilter()
{
	GibCore::ImageFilterParams filterInfo;
	char imgfilterbuffer[sizeof(GibCore::ImageFilterParams)];
	recvfrom(serverSocket, imgfilterbuffer, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&client, &slen);
	memcpy(&filterInfo, imgfilterbuffer, sizeof(GibCore::ImageFilterParams));
	return filterInfo;
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
		break;
	case GibCore::ImageFilter::FLIPPING:
		break;
	case GibCore::ImageFilter::BRIGHTNESSADJUST:
		break;
	case GibCore::ImageFilter::CONTRASTADJUST:
		break;
	case GibCore::ImageFilter::GAMMACORRECTION:
		break;
	case GibCore::ImageFilter::CHANGECOLORSPACE:
		break;
	case GibCore::ImageFilter::GAUSSIANBLUR:
		break;
	case GibCore::ImageFilter::BOXBLUR:
		break;
	case GibCore::ImageFilter::SHARPENING:
		break;
	default:
		return "Unknown";
	}
}

void UDPServer::CloseAndCleanup()
{
	closesocket(serverSocket);
	WSACleanup();
}

#include "UDPClient.h"

bool UDPClient::Init(int port, std::string ip)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}

	memset((char*)&server, 0, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip.c_str());

	return true;
}

bool UDPClient::CreateSocket()
{
	if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool UDPClient::LoadImageFromPath(std::string path, cv::Mat& img, std::string& extension)
{
	// Checks if file exists and then loads the file
	std::ifstream fstream(path.c_str());
	int ext = strlen(path.c_str());
	for (int i = ext - 4; i < ext; i++)
	{
		extension += path[i];
	}
	std::cout << extension<< std::endl;
	if (!fstream.good()) return false;
	img = cv::imread(path);	
}



void UDPClient::SendImage(cv::Mat& img, std::string extension)
{
	std::vector<uchar> buf;
	cv::imencode(extension, img, buf);
	size_t val = buf.size();
	char sizeButChar[sizeof(size_t)];
	memcpy(sizeButChar, &val, sizeof(size_t));
	int bufSize = sizeof(size_t);
	sendto(clientSocket, sizeButChar, bufSize, 0, (sockaddr*)&server, sizeof(sockaddr_in));

	char portChar[sizeof(int)];
	sockaddr_in fromSock;
	int slen = sizeof(sockaddr_in);
	std::cout << "recieving\n";
	//std::this_thread::sleep_for(std::chrono::milliseconds(50));
	recvfrom(clientSocket, portChar, sizeof(int),0, (sockaddr*)&fromSock, &slen);
	int port;
	memcpy(&port, &portChar, sizeof(int));
	std::cout << "New port - " << port << std::endl;
	server.sin_port = htons(port);
	size_t remainingToSend = buf.size();
	uchar* from = &buf[0];
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	while (remainingToSend > 0)
	{
		size_t sendSize = remainingToSend > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToSend;
		while (sendto(clientSocket, (const char*)from, sendSize, 0, (const struct sockaddr*)&server, sizeof(server)) < 0)
		{
			printf("Error during send, retrying...\n");
		}
		remainingToSend -= sendSize;
		from += sendSize;
		std::cout << remainingToSend << " - What is left to send" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void UDPClient::SendFilter(GibCore::ImageFilterParams params)
{
	std::cout << "Sending Filter Info\n";
	char newMsg[sizeof(GibCore::ImageFilterParams)];
	memset(newMsg, 0, sizeof(GibCore::ImageFilterParams));
	memcpy(newMsg, &params, sizeof(GibCore::ImageFilterParams));
	//paramsS.params = std::string("Hello").c_str();
	sendto(clientSocket, (const char*)&params, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&server, sizeof(sockaddr_in));
}

bool UDPClient::VerifyImage()
{
	int slen = sizeof(sockaddr_in);
	char textBuffer[1024];
	if (recvfrom(clientSocket, (char*)textBuffer, 1024, 0, (sockaddr*)&server, &slen) == SOCKET_ERROR)
	{
		printf("Failed receiving message back");
		return false;
	}
	return true;
}

void UDPClient::CloseAndCleanup()
{
	closesocket(clientSocket);
	WSACleanup();
}

GibCore::ImageFilter UDPClient::FilterFromString(std::string str)
{
	//std::cout << str << std::endl;
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return std::tolower(c); });

	if (str == "resize") return GibCore::ImageFilter::RESIZE;
	else if (str == "rotation") return GibCore::ImageFilter::ROTATION;
	else if (str == "cropping") return GibCore::ImageFilter::CROPPING;
	else if (str == "flipping") return GibCore::ImageFilter::FLIPPING;
	else if (str == "brightness") return GibCore::ImageFilter::BRIGHTNESSADJUST;
	else if (str == "contrast") return GibCore::ImageFilter::CONTRASTADJUST;
	else if (str == "gamma") return GibCore::ImageFilter::GAMMACORRECTION;
	else if (str == "colour-space") return GibCore::ImageFilter::CHANGECOLORSPACE;
	else if (str == "gaussianblur") return GibCore::ImageFilter::GAUSSIANBLUR;
	else if (str == "boxblur") return GibCore::ImageFilter::BOXBLUR;
	else if (str == "sharpening") return GibCore::ImageFilter::SHARPENING;
	else return GibCore::ImageFilter::NOTHING;
	//return GibCore::ImageFilter::RESIZE;
}
/*
enum class ImageFilter
{
	RESIZE,
	ROTATION,
	CROPPING,
	FLIPPING,
	BRIGHTNESSADJUST,
	CONTRASTADJUST,
	GAMMACORRECTION,
	CHANGECOLORSPACE,
	GAUSSIANBLUR,
	BOXBLUR,
	SHARPENING
};
*/

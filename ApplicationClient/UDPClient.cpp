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
	u_long blockMode = 0;
	if (ioctlsocket(clientSocket, FIONBIO, &blockMode) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}

bool UDPClient::LoadImageFromPath(std::string path, cv::Mat& img, std::string& extension)
{
	// Checks if file exists and then loads the 
	std::cout << "img path - " << path << std::endl;
	std::ifstream fstream(path.c_str());
	int ext = strlen(path.c_str());
	for (int i = ext - 4; i < ext; i++)
	{
		extension += path[i];
	}
	std::cout << extension<< std::endl;
	if (!fstream.good()) return false;
	img = cv::imread(path);
	if (img.empty()) return false;
	return true;
}



void UDPClient::SendImage(cv::Mat& img, std::string extension)
{
	std::vector<uchar> buf;
	cv::imencode(extension, img, buf);
	size_t val = buf.size();
	char sizeButChar[sizeof(size_t)];
	memcpy(sizeButChar, &val, sizeof(size_t));
	int bufSize = sizeof(size_t);
	//mutex.lock();
	if (sendto(clientSocket, sizeButChar, bufSize, 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		std::cout << "Sendto error on sending size. Error code: " << WSAGetLastError() << std::endl;
		ExitProgram("Problem sending image");
	}
	char verificationChar[sizeof(int)];
	int verification;
	sockaddr_in fromSock;
	int slen = sizeof(sockaddr_in);
	if (recvfrom(clientSocket, (char*)verificationChar, sizeof(int), 0, (sockaddr*)&fromSock, &slen) == SOCKET_ERROR)
	{
		std::cout << "Recvfrom error. Error code: " << WSAGetLastError() << std::endl;
		ExitProgram("Server connection reset");
	}
	memcpy(&verification, &verificationChar, sizeof(int));
	if (verification == 0)
	{
		ExitProgram("Too many clients connected to the server. Please try again later");
	}

	char portChar[sizeof(int)];
	
	std::cout << "recieving\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	
	if (recvfrom(clientSocket, portChar, sizeof(int), 0, (sockaddr*)&fromSock, &slen) == SOCKET_ERROR)
	{
		std::cout << "Recvfrom error. Error code: " << WSAGetLastError() << std::endl;
		ExitProgram("Server connection reset");
	}
	//mutex.unlock();
	int port;
	memcpy(&port, &portChar, sizeof(int));
	std::cout << "New port - " << port << std::endl;
	sockaddr_in tempServer;
	memset((char*)&tempServer, 0, sizeof(tempServer));

	tempServer.sin_family = AF_INET;
	tempServer.sin_port = htons(port);
	tempServer.sin_addr.s_addr = server.sin_addr.s_addr;
	server.sin_port = htons(port);

	size_t remainingToSend = buf.size();
	uchar* from = &buf[0];
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	while (remainingToSend > 0)
	{
		size_t sendSize = remainingToSend > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToSend;
		while (sendto(clientSocket, (const char*)from, sendSize, 0, (const struct sockaddr*)&tempServer, sizeof(tempServer)) < 0)
		{
			printf("Error during send, retrying...\n");
		}
		remainingToSend -= sendSize;
		from += sendSize;
		std::cout << remainingToSend << " - What is left to send" << std::endl;
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	bool verified = VerifyImage(img, clientSocket, tempServer);
	std::cout << "value of verified - " << verified << std::endl;
}

void UDPClient::SendImageMultiThreaded(cv::Mat& img, std::string extension, cv::Mat& img2, std::string extension2)
{
	std::thread si1(&UDPClient::SendImage, this, std::ref(img), extension);
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	std::thread si2(&UDPClient::SendImage, this, std::ref(img2), extension2);
	si1.join();
	si2.join();
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


bool UDPClient::VerifyImage(cv::Mat& img, SOCKET& clientSocket, sockaddr_in& tempServer)
{
	double hash = GibCore::CalculateHash(img);
	sendto(clientSocket, (char*)&hash, sizeof(double), 0, (sockaddr*)&tempServer, sizeof(sockaddr_in));
	char verified[sizeof(bool)];
	sockaddr_in received;
	int slen = sizeof(sockaddr_in);
	recvfrom(clientSocket, verified, sizeof(bool), 0, (sockaddr*)&received, &slen);
	bool conv;
	memcpy(&conv, verified, sizeof(bool));
	return conv;
}

cv::Mat UDPClient::ReceieveImage(SOCKET& clientSocket)
{
	std::cout << "receiving image";
	sockaddr_in server;
	char sizeBuf[sizeof(size_t)];
	int slen = sizeof(sockaddr_in);
	recvfrom(clientSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&server, &slen);

	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));

	char* buffer = new char[actualSize];
	std::cout << "Parallel actual size - " << actualSize << std::endl;
	size_t remainingToReceieve = actualSize;

	char* bufferPos = &buffer[0];
	while (remainingToReceieve > 0)
	{
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		//sockaddr_in newClient;
		//std::cout << "Gets here\n";
		while (recvfrom(clientSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&server, &slen) == SOCKET_ERROR)
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
	imgData.assign(buffer, buffer + actualSize);
	image = cv::imdecode(cv::Mat(imgData), 1);
	delete[] buffer;

	cv::imshow("client img", image);
	cv::waitKey(0);
	cv::destroyWindow("client img");

	return image;
}

void UDPClient::SaveImage(cv::Mat& img, std::string nameAndPath)
{
	cv::imwrite(nameAndPath, img);
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
	else if (str == "rotation" || str == "rotate") return GibCore::ImageFilter::ROTATION;
	else if (str == "cropping" || str == "crop") return GibCore::ImageFilter::CROPPING;
	else if (str == "flipping" || str == "flip") return GibCore::ImageFilter::FLIPPING;
	else if (str == "brightness") return GibCore::ImageFilter::BRIGHTNESSADJUST;
	else if (str == "contrast") return GibCore::ImageFilter::CONTRASTADJUST;
	else if (str == "gamma") return GibCore::ImageFilter::GAMMACORRECTION;
	else if (str == "hsv") return GibCore::ImageFilter::TOHSV;
	else if (str == "grayscale") return GibCore::ImageFilter::TOGREYSCALE;
	else if (str == "gaussianblur") return GibCore::ImageFilter::GAUSSIANBLUR;
	else if (str == "boxblur") return GibCore::ImageFilter::BOXBLUR;
	else if (str == "sharpening" || str == "sharpen") return GibCore::ImageFilter::SHARPENING;
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

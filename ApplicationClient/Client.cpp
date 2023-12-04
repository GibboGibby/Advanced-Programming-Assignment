#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "..\ApplicationServer\core.h"
#include <iostream>

#include <chrono>
#include <thread>





int main()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return 1;
	}

	sockaddr_in server;
	SOCKET clientSocket;

	if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
	{
		return 2;
	}

	memset((char*)&server, 0, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = inet_addr(SERVER);

	while (true)
	{
		char message[sizeof GibCore::SentStruct];
		memset(message, 0, sizeof(GibCore::SentStruct));


		cv::Mat img = cv::imread("./bad-yuumi.jpg");
		std::vector<uchar> buf;
		//buf.resize(200 * 1024 * 1024);
		cv::imencode(".jpg", img, buf);
		
		GibCore::SentStruct structToSend;
		structToSend.name = "Jame Gib";
		structToSend.id = 69;
		structToSend.vec = { 12,19 };
		structToSend.theClass.SetVal(420);
		structToSend.imgSize = img.rows * img.cols * img.channels();
		structToSend.imgSizeAsUCharPtr = img.data;

		memcpy(message, &structToSend, sizeof(structToSend));

		const char* dataInBytes = reinterpret_cast<char*>(&structToSend);

		std::cout << buf.size() << std::endl;

		GibCore::Image imgStruct;
		imgStruct.imgData = (char*) & buf[0];
		imgStruct.imgSize = buf.size();

		std::cout << buf.size() << " - Regular" << std::endl;
		size_t val = buf.size();
		char sizeButChar[sizeof(size_t)];
		memcpy(sizeButChar, &val, sizeof(size_t));
		int bufSize = sizeof(size_t);
		sendto(clientSocket, sizeButChar, bufSize, 0, (sockaddr*)&server, sizeof(sockaddr_in));


		size_t remainingToSend = buf.size();
		uchar* from = &buf[0];
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
		}
		/*

		if (sendto(clientSocket, (const char*)&buf[0], buf.size(), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			std::cout << "Sending error\n" << WSAGetLastError() << std::endl;
			return 3;
		}
		
		*/

		/*
		if (sendto(clientSocket, dataInBytes, sizeof(message), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			return 3;
		}
		*/
		int slen = sizeof(sockaddr_in);
		char textBuffer[1024];
		if (recvfrom(clientSocket, (char*)textBuffer, 1024, 0, (sockaddr*)&server, &slen) == SOCKET_ERROR)
		{
			printf("Failed receiving message back");
		}

		printf("Buffer value: %s\n", textBuffer);



		break;
	}
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}

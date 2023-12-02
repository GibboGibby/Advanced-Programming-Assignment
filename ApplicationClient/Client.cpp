#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "..\ApplicationServer\core.h"
#include <iostream>




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
	server.sin_addr.s_addr = inet_addr("127.0.0.1");

	while (true)
	{
		char message[sizeof GibCore::SentStruct];
		memset(message, 0, sizeof(GibCore::SentStruct));


		cv::Mat img = cv::imread("C:/Users/james/Pictures/Screenshots/bad-yuumi.jpg");
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

		std::cout << buf.size();

		if (sendto(clientSocket, (const char*)&buf[0], buf.size(), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			return 3;
		}

		/*
		if (sendto(clientSocket, dataInBytes, sizeof(message), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			return 3;
		}
		*/
		break;
	}
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}

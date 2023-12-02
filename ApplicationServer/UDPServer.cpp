#include "UDPServer.h"

bool UDPServer::Init()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		return false;
	}

	if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		return false;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (bind(serverSocket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		return false;
	}

	while (true)
	{
		printf("Waiting for data");

		fflush(stdout);

		GibCore::SentStruct receivedInfo;

		//char buffer[sizeof(GibCore::SentStruct)];
		char buffer[60547];


		int len;
		int slen = sizeof(sockaddr_in);
		//if (len = recvfrom(serverSocket, buffer, sizeof(GibCore::SentStruct), 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
		if (len = recvfrom(serverSocket, buffer, 60547, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
		{
			printf("RecvFrom Failed!\n");
		}

		//memcpy(&receivedInfo, buffer, sizeof(receivedInfo));
		/*
		// Attempting struct stuff
		//memcpy(buffer, &receivedInfo, sizeof(receivedInfo));
		memcpy(&receivedInfo, &buffer, sizeof(buffer));
		std::cout << "Received data packet of the struct with these values: " << std::endl;

		std::cout << "Name: " << receivedInfo.name << std::endl;
		std::cout << "ID: " << receivedInfo.id << std::endl;
		std::cout << "Vec x: " << receivedInfo.vec.x << std::endl;
		std::cout << "Vec y: " << receivedInfo.vec.y << std::endl;

		std::cout << "Class value: " << receivedInfo.theClass.GetVal() << std::endl;
		*/

		cv::namedWindow("ServerDisplay", cv::WINDOW_AUTOSIZE);
		cv::Mat image;
		std::vector<uchar> vec;
		vec.assign(buffer, buffer + 60547);

		image = cv::imdecode(cv::Mat(vec), 1);
		//image = cv::imdecode(cv::Mat(1, receivedInfo.imgSize * sizeof(uchar), CV_8UC1, receivedInfo.imgAsUChar), cv::IMREAD_UNCHANGED);
		cv::imshow("ServerDisplay", image);
		cv::waitKey(0);
		cv::destroyWindow("ServerDisplay");

	}
	// https://gist.github.com/sunmeat/02b60c8a3eaef3b8a0fb3c249d8686fd
	// This is a great resource for learning.
	// Need to make the client a class and tidy this one up so not everything is done in the Init method
	// Struct sends good tho
	// Now need to test it with an opencv image
}
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
		printf("Waiting for data\n");

		fflush(stdout);

		GibCore::SentStruct receivedInfo;

		//char buffer[sizeof(GibCore::SentStruct)];


		int len;
		int slen = sizeof(sockaddr_in);
		
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
			while (len = recvfrom(serverSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
			{
				printf("RecvFrom Failed!\nThis reason: %s\n", WSAGetLastError());
				printf("But this is being retried\n");
			}
			remainingToReceieve -= sendSize;
			bufferPos += sendSize;
			std::cout << "This is the amount received - " << remainingToReceieve << std::endl;
		}
		

		std::cout << "Makes it here, thus receiving all of the data\n";

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

		char message[1024] = { "Recieved ur image good" };

		if (sendto(serverSocket, message, 1024, 0, (sockaddr*)&client, sizeof(sockaddr_in)) == SOCKET_ERROR)
		{
			printf("Sendto error\n");
		}



		cv::namedWindow("ServerDisplay", cv::WINDOW_AUTOSIZE);
		cv::Mat image;
		std::vector<uchar> vec;
		vec.assign(buffer, buffer + actualSize);

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

	// https://stackoverflow.com/questions/57794550/sending-large-files-over-udp
	// This is a great resource for learning how to send larger files across UDP
	// Seems quite simple just requires to split teh packet into a lot of smaller packets
	// I think recv from should let me receive all the packets all I need is to alter the send on the client side
	// As this is where the error is coming from
}
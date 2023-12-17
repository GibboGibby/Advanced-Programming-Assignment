#include <iostream>
#include "UDPServer.h"

int main()
{


	std::cout << "Hello from server" << std::endl;
	
	UDPServer server;
	if (!server.Init())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	if (!server.CreateSocket())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}

	if (!server.BindServer())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		server.CloseAndCleanup();
	}

	while (true)
	{
		cv::Mat image = server.ReceiveImage();
		GibCore::ImageFilterParams filterInfo = server.RecieveFilter();

		std::cout << "filter info filter - " << server.GetEnumFilterName(filterInfo.filter) << std::endl;
		std::stringstream ss(filterInfo.params);
		std::vector<std::string> parameters;
		std::string temp;
		while (ss >> temp)
			parameters.push_back(temp);

		for (std::string str : parameters)
		{
			std::cout << str << std::endl;
		}
		std::cout << std::endl;

		cv::imshow("Display", image);
		cv::waitKey(0);
		cv::destroyWindow("Display");
	}

	return 0;
} 
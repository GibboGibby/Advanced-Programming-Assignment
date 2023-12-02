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

	return 0;
} 
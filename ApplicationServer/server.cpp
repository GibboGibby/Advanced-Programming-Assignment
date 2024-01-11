#include <iostream>
#include "UDPServer.h"

int main()
{


	std::cout << "Hello from server" << std::endl;
	// Create server object
	UDPServer server;
	// Init server
	if (!server.Init())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}
	// Create Socket
	if (!server.CreateSocket())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return 1;
	}
	// Bind Server
	if (!server.BindServer())
	{
		std::cerr << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		server.CloseAndCleanup();
		return 1;
	}
	
	while (true)
	{
		server.ReceiveImageParallel();
	}

	server.CloseAndCleanup();

	return 0;
} 
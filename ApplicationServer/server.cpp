#include <iostream>
#include "UDPServer.h"

int main(int argc, char** argv)
{
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

	std::cout << "Server created and bound to port and now waiting for packets" << std::endl;

	
	while (true)
	{
		server.ReceiveImageParallel();
	}

	server.CloseAndCleanup();

	return 0;
} 
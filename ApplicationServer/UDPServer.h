#pragma once
#include "Core.h"
#include <iostream>
class UDPServer
{
private:
	SOCKET serverSocket;
	sockaddr_in server, client;
public:
	bool Init();
};
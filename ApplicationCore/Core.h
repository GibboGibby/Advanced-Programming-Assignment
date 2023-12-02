#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <string>
#include <vector>

#define SERVER "127.0.0.1"
#define PORT 8888


namespace GibCore
{

	class InsideClass
	{
	private:
		int val = 52;

	public:
		void SetVal(int num) { val = num; }
		int GetVal() { return val; }
	};

	struct InsideSentStruct
	{
		int x, y;
	};

	struct SentStruct
	{
		std::string name;
		int id;
		InsideSentStruct vec;
		InsideClass theClass;
	};



}
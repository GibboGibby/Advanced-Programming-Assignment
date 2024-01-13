#include "UDPClient.h"

bool UDPClient::Init(int port, std::string ip)
{
	// Initialise WSA
	if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0)
	{
		return false;
	}
	// Set server to 0s
	memset((char*)&_server, 0, sizeof(_server));

	// Set server info that has been specified in the arguments supplied to the program
	_server.sin_family = AF_INET;
	_server.sin_port = htons(port);
	_server.sin_addr.s_addr = inet_addr(ip.c_str());
	// Returns true if initialising was successful
	return true;
}

bool UDPClient::CreateSocket()
{
	// Creates the socket for the  client (Specifying SOCK_DGRAM for UDP)
	if ((_clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
	{
		return false;
	}
	// Sets the blocking mode to ensure that the socket blocks
	u_long blockMode = 0;
	if (ioctlsocket(_clientSocket, FIONBIO, &blockMode) == SOCKET_ERROR)
	{
		return false;
	}
	// Set socket timeout
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 10000000;
	if (setsockopt(_clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv.tv_usec, sizeof(long)) == SOCKET_ERROR)
		return false;
	return true;
}

/// <summary>
/// Takes a path and loads the image and extension into the references params
/// </summary>
/// <param name="path">path to file</param>
/// <param name="img">Reference to image that the image will be loaded into</param>
/// <param name="extension">Refernce to extension that the extension will be loaded into</param>
/// <returns>bool whether the function was successful</returns>
bool UDPClient::LoadImageFromPath(const std::string& path, cv::Mat& img, std::string& extension)
{
	// Checks if file exists and then loads the 
	std::cout << "img path - " << path << std::endl;
	std::ifstream fstream(path.c_str());
	//size_t pos = path.find(".");
	//extension = path.substr(pos);

	std::string tempExt = "";
	
	for (int i = strlen(path.c_str()); i > 0; i--)
	{
		tempExt += path[i];
		if (path[i] == '.') break;
	}
	std::reverse(tempExt.begin(), tempExt.end());
	extension = tempExt;
	std::cout << extension << " - new ext" << std::endl;
	/*
	int ext = strlen(path.c_str());
	for (int i = ext - 4; i < ext; i++)
	{
		extension += path[i];
	}
	*/
	std::cout << extension<< std::endl;
	if (!fstream.good()) return false;
	img = cv::imread(path);
	if (img.empty()) return false;
	return true;
}

void UDPClient::ExitProgram(std::string extra)
{
	if (extra == "")
		std::cout << "A problem has occured program will now terminate" << std::endl;
	else
		std::cout << "Error: " << extra << " - Program will now Terminate" << std::endl;
	WSACleanup();
	closesocket(_clientSocket);
	exit(EXIT_FAILURE);
}


/// <summary>
/// Sends Image to the Server
/// </summary>
/// <param name="img">Image to send</param>
/// <param name="extension">extension of image</param>
void UDPClient::SendImage(cv::Mat& img, std::string extension)
{
	// Create a buffer for the image values
	std::vector<uchar> buf;
	// Load image data into the buf
	cv::imencode(extension, img, buf);
	// Gets the image size in bytes
	size_t val = buf.size();
	// Create a char array of the size and memcpy the value into it
	char sizeButChar[sizeof(size_t)];
	memcpy(sizeButChar, &val, sizeof(size_t));
	int bufSize = sizeof(size_t);
	
	// Send size to the server
	if (sendto(_clientSocket, sizeButChar, bufSize, 0, (sockaddr*)&_server, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		std::cout << "Sendto error on sending size. Error code: " << WSAGetLastError() << std::endl;
		ExitProgram("Problem sending image");
	}
	// Creates char and recvs from the server
	char verificationChar[sizeof(int)];
	int verification;
	sockaddr_in fromSock;
	int slen = sizeof(sockaddr_in);
	if (recvfrom(_clientSocket, (char*)verificationChar, sizeof(int), 0, (sockaddr*)&fromSock, &slen) == SOCKET_ERROR)
	{
		std::cout << "Recvfrom error. Error code: " << WSAGetLastError() << std::endl;
		ExitProgram("Server connection reset");
	}
	// Copyies the char recieved into the verification int
	memcpy(&verification, &verificationChar, sizeof(int));
	// Checks to see if the client is accepted by the server
	if (verification == 0)
	{
		ExitProgram("Too many clients connected to the server. Please try again later");
	}

	// Creates a char array of int bytes
	char portChar[sizeof(int)];
	std::cout << "recieving\n";
	// Receive into char array
	if (recvfrom(_clientSocket, portChar, sizeof(int), 0, (sockaddr*)&fromSock, &slen) == SOCKET_ERROR)
	{
		std::cout << "Recvfrom error. Error code: " << WSAGetLastError() << std::endl;
		ExitProgram("Server connection reset");
	}
	// Copy into port int
	int port;
	memcpy(&port, &portChar, sizeof(int));
	std::cout << "New port - " << port << std::endl;
	sockaddr_in tempServer;
	// Create and set a new server for the client to send to
	// Using the port recieved by client
	memset((char*)&tempServer, 0, sizeof(tempServer));
	tempServer.sin_family = AF_INET;
	tempServer.sin_port = htons(port);
	tempServer.sin_addr.s_addr = _server.sin_addr.s_addr;
	_server.sin_port = htons(port);

	// Get the size of the image and a pointer to the start of the vector
	size_t remainingToSend = buf.size();
	uchar* from = &buf[0];
	// Loops while there is still image data to send
	while (remainingToSend > 0)
	{
		// Gets the send size based on the remaining data to send
		// If the data is smaller than the set size the smaller size is sent
		size_t sendSize = remainingToSend > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToSend;
		// Sends the data to the server from the vector pointer and based on the size calculated above
		while (sendto(_clientSocket, (const char*)from, sendSize, 0, (const struct sockaddr*)&tempServer, sizeof(tempServer)) < 0)
		{
			printf("Error during send, retrying...\n");
		}
		// Decreases remaining to send by the amount send
		remainingToSend -= sendSize;
		// Increase pointer by the amount sent
		from += sendSize;
		std::cout << remainingToSend << " - What is left to send" << std::endl;
		
	}
	
	// Check to see if the image hash on both ends is the same;
	bool verified = VerifyImage(img, _clientSocket, tempServer);
	std::cout << "value of verified - " << verified << std::endl;
}

/// <summary>
/// Sends two images on two seperate threads. This is to check multi threading on the server
/// </summary>
/// <param name="img">First image to send</param>
/// <param name="extension"> first images extension</param>
/// <param name="img2">Second image to send</param>
/// <param name="extension2">Second Images extension</param>
void UDPClient::SendImageMultiThreaded(cv::Mat& img, std::string extension, cv::Mat& img2, std::string extension2)
{
	//Creates a send image thread
	std::thread si1(&UDPClient::SendImage, this, std::ref(img), extension);
	// Sleeps for 20 milliseconds (probably not required just made testing easier)
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	std::thread si2(&UDPClient::SendImage, this, std::ref(img2), extension2);
	si1.join();
	si2.join();
}
/// <summary>
/// Sends filter information to the server
/// First creates a char buffer with the size of the struct
/// Sets all the values to zero
/// Copies the value in and then sends it
/// </summary>
/// <param name="params">Struct containing the parameters</param>
void UDPClient::SendFilter(GibCore::ImageFilterParams params)
{
	std::cout << "Sending Filter Info\n";
	char newMsg[sizeof(GibCore::ImageFilterParams)];
	memset(newMsg, 0, sizeof(GibCore::ImageFilterParams));
	memcpy(newMsg, &params, sizeof(GibCore::ImageFilterParams));
	//paramsS.params = std::string("Hello").c_str();
	sendto(_clientSocket, (const char*)&params, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&_server, sizeof(sockaddr_in));
}

/// <summary>
/// Verifies the image by first calculating a hash, sending it to the server and receiving whether it is the same
/// </summary>
/// <param name="img">Image to get hash from</param>
/// <param name="clientSocket">Socket to send and receive from</param>
/// <param name="tempServer">Server address</param>
/// <returns>Whether the hash is close enough to the value</returns>
bool UDPClient::VerifyImage(cv::Mat& img, SOCKET& clientSocket, sockaddr_in& tempServer)
{
	double hash = GibCore::CalculateHash(img);
	sendto(clientSocket, (char*)&hash, sizeof(double), 0, (sockaddr*)&tempServer, sizeof(sockaddr_in));
	char verified[sizeof(bool)];
	sockaddr_in received;
	int slen = sizeof(sockaddr_in);
	recvfrom(clientSocket, verified, sizeof(bool), 0, (sockaddr*)&received, &slen);
	bool conv;
	memcpy(&conv, verified, sizeof(bool));
	return conv;
}

/// <summary>
/// Receive an image from the socket
/// </summary>
/// <param name="clientSocket">Client socket</param>
/// <returns>Image that has been received from the socket</returns>
cv::Mat UDPClient::ReceieveImage(SOCKET& clientSocket)
{
	std::cout << "receiving image";
	sockaddr_in server;
	char sizeBuf[sizeof(size_t)];
	int slen = sizeof(sockaddr_in);
	recvfrom(clientSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&server, &slen);

	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));

	char* buffer = new char[actualSize];
	std::cout << "Parallel actual size - " << actualSize << std::endl;
	size_t remainingToReceieve = actualSize;

	char* bufferPos = &buffer[0];
	while (remainingToReceieve > 0)
	{
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		//sockaddr_in newClient;
		//std::cout << "Gets here\n";
		while (recvfrom(clientSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&server, &slen) == SOCKET_ERROR)
		{
			printf("RecvFrom Failed!\nThis reason: %i\n", WSAGetLastError());
			printf("But this is being retried\n");
		}
		remainingToReceieve -= sendSize;
		bufferPos += sendSize;
		std::cout << "This is the amount to be received - " << remainingToReceieve << std::endl;

	}
	cv::Mat image;
	std::vector<uchar> imgData;
	imgData.assign(buffer, buffer + actualSize);
	image = cv::imdecode(cv::Mat(imgData), 1);
	delete[] buffer;

	cv::imshow("client img", image);
	cv::waitKey(0);
	cv::destroyWindow("client img");

	return image;
}

bool UDPClient::CheckImageSuccessfullyFiltered(SOCKET& socket)
{
	sockaddr_in from;
	int sockaddrSize = sizeof(sockaddr_in);
	bool success;
	char successChar[sizeof(bool)];
	recvfrom(socket, (char*)successChar, sizeof(bool), 0, (sockaddr*)&from, &sockaddrSize);
	memcpy(&success, successChar, sizeof(bool));

	if (success == true) return true;

	char errorMsg[1024];
	recvfrom(socket, (char*)errorMsg, 1024, 0, (sockaddr*)&from, &sockaddrSize);
	std::cout << errorMsg << std::endl;
	return false;
}

/// <summary>
/// Saves the image to disk
/// </summary>
/// <param name="img">Image to save</param>
/// <param name="nameAndPath">Path to save to</param>
void UDPClient::SaveImage(cv::Mat& img, std::string nameAndPath)
{
	cv::imwrite(nameAndPath, img);
}

/// <summary>
/// Close the socket and run WSACleanup
/// </summary>
void UDPClient::CloseAndCleanup()
{
	closesocket(_clientSocket);
	WSACleanup();
}

/// <summary>
/// Takes in a string and converts it to the correct filter enum
/// </summary>
/// <param name="str">String to convert</param>
/// <returns>ImageFilter enum</returns>
GibCore::ImageFilter UDPClient::FilterFromString(std::string str)
{
	//std::cout << str << std::endl;
	// Converts string to lowercase for easy comparing
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return std::tolower(c); });

	// If statement to check and return enum
	if (str == "resize") return GibCore::ImageFilter::RESIZE;
	else if (str == "rotation" || str == "rotate") return GibCore::ImageFilter::ROTATION;
	else if (str == "cropping" || str == "crop") return GibCore::ImageFilter::CROPPING;
	else if (str == "flipping" || str == "flip") return GibCore::ImageFilter::FLIPPING;
	else if (str == "brightness") return GibCore::ImageFilter::BRIGHTNESSADJUST;
	else if (str == "contrast") return GibCore::ImageFilter::CONTRASTADJUST;
	else if (str == "gamma") return GibCore::ImageFilter::GAMMACORRECTION;
	else if (str == "hsv") return GibCore::ImageFilter::TOHSV;
	else if (str == "grayscale") return GibCore::ImageFilter::TOGREYSCALE;
	else if (str == "gaussianblur") return GibCore::ImageFilter::GAUSSIANBLUR;
	else if (str == "boxblur") return GibCore::ImageFilter::BOXBLUR;
	else if (str == "sharpening" || str == "sharpen") return GibCore::ImageFilter::SHARPENING;
	else return GibCore::ImageFilter::NOTHING;
	
}

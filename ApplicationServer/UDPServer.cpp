#include "UDPServer.h"

bool UDPServer::Init()
{
	// Init winsock
	if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0)
	{
		return false;
	}
	// Set values of server to 0 so that there are no errors
	memset((char*)&_server, 0, sizeof(_server));
	// Set the server values so that packets from any address can be taken from the target port
	_server.sin_family = AF_INET;
	_server.sin_addr.s_addr = INADDR_ANY;
	_server.sin_port = htons(PORT);
	

	return true;
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

bool UDPServer::CreateSocket()
{
	// Create the socket
	if ((_serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		return false;
	}
	// Lets the address be reused for the multi port
	int optval = 1;
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) & optval, sizeof(optval));
	return true; 
}

bool UDPServer::BindServer()
{
	// Binds the server socket to the specific server information
	if (bind(_serverSocket, (sockaddr*)&_server, sizeof(_server)) == SOCKET_ERROR)
	{
		return false;
	}
	// Adds the default port to the list of used ports
	_usedPorts.push_back(8888);
	return true;
}

/// <summary>
/// Receives the image parallel-ly
/// </summary>
void UDPServer::ReceiveImageParallel()
{
	// Setting values to store the image and to wait for a packet send from a client
	sockaddr_in newClient;
	char sizeBuf[sizeof(size_t)];
	// Receieves from a client into the char buf
	recvfrom(_serverSocket, sizeBuf, sizeof(size_t), 0, (sockaddr*)&newClient, &_slen);
	// Checks to see if the number of ports (which is the number of clients) and checks if another thread can be made
	int confirmation = 1;
	if (_usedPorts.size() - 1 >= SERVER_THREADS)
	{
		confirmation = 0;
		std::cout << "Too many clients currently connecting. refusing connection";
	}
	// Sends the confirmation to the server informing the client if there is a thread for them
	sendto(_serverSocket, (char*)&confirmation, sizeof(int), 0, (sockaddr*)&newClient, _slen);
	//Exits if there is no space for that client
	if (confirmation == 0) return;
	// Copies the size into a size_t variable
	size_t actualSize;
	memcpy(&actualSize, sizeBuf, sizeof(size_t));
	// Creates a new port for the server to use based on the last port opened
	int port = _usedPorts[_usedPorts.size() - 1] + 1;
	
	std::cout << "Now sending info back" << std::endl;
	// Adds port to list
	_usedPorts.push_back(port);
	std::cout << "Size just after receive - " << actualSize << std::endl;
	// Creates thread passing the port and the size. This thread receieves the image and processes
	std::thread thr(&UDPServer::ReceivingAndProcessing, this, newClient, actualSize, port);
	// Detaches thread from the main thread since it handles everything
	thr.detach();
}
/// <summary>
/// Does the receiving and processing of the image
/// Receives image, receives filter and parameters
/// Performs the filters on the image
/// Sends it back to the client
/// </summary>
/// <param name="client">Client address</param>
/// <param name="size">Size of the image</param>
/// <param name="port">Port the server should use</param>
void UDPServer::ReceivingAndProcessing(sockaddr_in client, size_t size, int port)
{
	//std::cout << GetCurrentThreadId() << " - " << client.sin_addr.S_un.S_addr << std::endl;
	std::cout << "\nCreating new thread on port - "<< port << std::endl;
	// Sends the port to the client
	sendto(_serverSocket, (const char*)&port, sizeof(int), 0, (sockaddr*)&client, _slen);
	// Creates a new UDP socket for the thread with the new port
	SOCKET threadSocket;
	threadSocket = socket(AF_INET, SOCK_DGRAM, 0);
	int optval = 1;
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
	sockaddr_in threadServer;
	memset((char*)&threadServer, 0, sizeof(threadServer));
	threadServer.sin_family = AF_INET;
	threadServer.sin_addr.s_addr = _server.sin_addr.s_addr;
	threadServer.sin_port = htons(port);

	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 10000000;
	if (setsockopt(threadSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv.tv_usec, sizeof(long)) == SOCKET_ERROR)
	{
		std::cout << "Error setting timeout. error code - " << WSAGetLastError() << std::endl;
		// Shuts down thread if the server fails to set the timeout
		TerminateThread(threadSocket, port);
		return;
	}

	// Binds thread socket to the new port
	if (bind(threadSocket, (sockaddr*)&threadServer, sizeof(threadServer)) == SOCKET_ERROR)
	{
		std::cout << "Error binding another socket. error code - " << WSAGetLastError() << std::endl;
		// Shuts down thread if the server fails to bind
		TerminateThread(threadSocket, port);
		return;
	}
	std::cout << "Server bound successfully\n";
	
	// Receives image
	cv::Mat image = ReceiveImage(threadSocket, size);

	// Verifies the image hash
	bool verified = VerifyImage(image, threadSocket, client);
	std::cout << "Verified value - " << verified << std::endl;
	// Terminates the thread if the image is not the same
	if (verified == 0)
	{
		std::cout << "Image has not been recieved properly. Please try again" << std::endl;
		TerminateThread(threadSocket, port);
		return;
	}

	// Receives filter information
	GibCore::ImageFilterParams params = ReceieveFilter(threadSocket);
	// Filters image based on the filter information sent

	cv::Mat filteredImage;
	try {
		filteredImage = FilterImage(image, params);

	}
	catch (GibException* e)
	{
		std::cout << e << std::endl;
		bool success = false;
		char successChar[sizeof(bool)];
		memcpy(&successChar, &success, sizeof(bool));
		sendto(threadSocket, (char*)successChar, sizeof(bool), 0, (sockaddr*)&client, _slen);
		sendto(threadSocket, (char*)e->OutputError().c_str(), 1024, 0, (sockaddr*)&client, _slen);
		TerminateThread(threadSocket, port);
		return;
		// Send false to client (with message) and then terminate the thread
	}
	bool success = true;
	char successChar[sizeof(bool)];
	memcpy(&successChar, &success, sizeof(bool));
	sendto(threadSocket, (char*)successChar, sizeof(bool), 0, (sockaddr*)&client, _slen);

	// Extension to output the image
	std::string ext = ".png";

	// Verify if the image filter succeeded
	// And maybe send the ext?

	// Send image back to the client
	SendImage(filteredImage, ext, threadSocket, client);

	// Verify image got there okay

	// Close thread
	TerminateThread(threadSocket, port);
}
//https://stackoverflow.com/questions/54155900/udp-server-and-connected-sockets
// For connect which should filter the packets. need to test on dans pc

/// <summary>
/// Receives the image from the client
/// </summary>
/// <param name="threadSocket">Reference to socket</param>
/// <param name="size">Size of image</param>
/// <returns>Received image</returns>
cv::Mat UDPServer::ReceiveImage(SOCKET& threadSocket, size_t size)
{
	sockaddr_in newClient;
	// Dynamically allocates space for the image
	// This is so the stack is not overloaded as well as because the image is not a set size at compile time
	char* buffer = new char[size];
	std::cout << "Parallel actual size - " << size << std::endl;
	// Gets the size and a pointer to the first element in the new image buffer
	size_t remainingToReceieve = size;
	char* bufferPos = &buffer[0];
	// Loops through the image size
	while (remainingToReceieve > 0)
	{
		// Gets the appropriate send size
		// This is just to ensure that the program doesn't get more than the image into the buffer
		size_t sendSize = remainingToReceieve > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToReceieve;
		// Receives the appropriate ammount ito the posiiton starting at the pointer to the start of the array
		while (recvfrom(threadSocket, (char*)bufferPos, UDP_BUF_SIZE, 0, (sockaddr*)&newClient, &_slen) == SOCKET_ERROR)
		{
			printf("RecvFrom Failed!\nThis reason: %i\n", WSAGetLastError());
			printf("But this is being retried\n");
		}
		// Decreases the amount to be sent
		remainingToReceieve -= sendSize;
		// Increases the position of the pointer
		bufferPos += sendSize;
		std::cout << "This is the amount to be received - " << remainingToReceieve << std::endl;

	}
	// Creates a vector of uchar to put the image into
	cv::Mat image;
	std::vector<uchar> imgData;
	// Inserts the data into the vector and then converts that vector into an image
	imgData.assign(buffer, buffer + size);
	image = cv::imdecode(cv::Mat(imgData), 1);
	// Frees the memory
	delete[] buffer;
	return image;
}
/// <summary>
/// Receives the filter and parameters from the client
/// </summary>
/// <returns>Struct of Filter Params</returns>
GibCore::ImageFilterParams UDPServer::ReceiveFilter()
{
	GibCore::ImageFilterParams filterInfo;
	// Creates char buffer for the data
	char imgfilterbuffer[sizeof(GibCore::ImageFilterParams)];
	recvfrom(_serverSocket, imgfilterbuffer, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&_client, &_slen);
	// Copies the char into the actual object
	memcpy(&filterInfo, imgfilterbuffer, sizeof(GibCore::ImageFilterParams));
	return filterInfo;
}


/// <summary>
/// Closes the socket and removes the port from the list
/// </summary>
/// <param name="socket">Socket</param>
/// <param name="port">Port</param>
void UDPServer::TerminateThread(SOCKET& socket, int& port)
{
	closesocket(socket);
	RemovePort(port);
}
/// <summary>
/// Converts char array into a vector of strings (delimited by spaces)
/// </summary>
/// <param name="params">ImageFilterParams - constains enum and char array</param>
/// <returns>Vector of strings</returns>
std::vector<std::string> UDPServer::GetVectorOfString(GibCore::ImageFilterParams params)
{
	std::stringstream ss(params.params);
	std::vector<std::string> parameters;
	std::string temp;
	while (ss >> temp)
		parameters.push_back(temp);

	return parameters;
}

/// <summary>
/// Turns the enum into a string
/// </summary>
/// <param name="filter">Enum filter</param>
/// <returns>Returns string based on enum</returns>
std::string UDPServer::GetEnumFilterName(GibCore::ImageFilter filter)
{
	switch (filter)
	{
	case GibCore::ImageFilter::RESIZE:
		return "Resize";
	case GibCore::ImageFilter::ROTATION:
		return "Rotation";
	case GibCore::ImageFilter::CROPPING:
		return "Cropping";
	case GibCore::ImageFilter::FLIPPING:
		return "Flipping";
	case GibCore::ImageFilter::BRIGHTNESSADJUST:
		return "BrightnessAdjust";
	case GibCore::ImageFilter::CONTRASTADJUST:
		return "ContrastAdjust";
	case GibCore::ImageFilter::GAMMACORRECTION:
		return "GammaCorrection";
	case GibCore::ImageFilter::TOHSV:
		return "HSV";
	case GibCore::ImageFilter::TOGREYSCALE:
		return "Greyscale";
	case GibCore::ImageFilter::GAUSSIANBLUR:
		return "GaussianBlur";
	case GibCore::ImageFilter::BOXBLUR:
		return "BoxBlur";
	case GibCore::ImageFilter::SHARPENING:
		return "Sharpening";
	default:
		return "Unknown";
	}
}
/// <summary>
/// Calculates the hash and then compares it with the clients hash. Once calculated the bool is sent back to the client
/// </summary>
/// <param name="img">Image</param>
/// <param name="threadSocket">Socket</param>
/// <param name="client">Client address</param>
/// <returns>If the image's hashes are close enough</returns>
bool UDPServer::VerifyImage(cv::Mat& img, SOCKET& threadSocket, sockaddr_in client)
{
	// Calculates hash
	double hash = GibCore::CalculateHash(img);
	char buf[sizeof(double)];
	sockaddr_in newClient;
	// Receives hash into char
	recvfrom(threadSocket, buf, sizeof(double), 0, (sockaddr*)&newClient, &_slen);
	double otherHash = 0;
	// Converts back to a double
	memcpy(&otherHash, buf, sizeof(double));
	// Checks if they are close enough, within an acceptable region
	bool isHash = GibCore::DoubleCloseEnough(hash, otherHash, HASH_ACCEPTABLE_ERROR);
	//_mutex.lock();
	// Sends bool back
	sendto(threadSocket, (char*) & isHash, sizeof(char), 0, (sockaddr*)&client, _slen);
	//_mutex.unlock();
	return isHash;
}

/// <summary>
/// Receives the filter and then converts it back to a struct
/// </summary>
/// <param name="threadSocket">Socket</param>
/// <returns>ImageFilterParams Struct</returns>
GibCore::ImageFilterParams UDPServer::ReceieveFilter(SOCKET& threadSocket)
{
	GibCore::ImageFilterParams params;
	char paramsBuffer[sizeof(GibCore::ImageFilterParams)];
	// Receieve
	sockaddr_in newCli;
	recvfrom(threadSocket, (char*)paramsBuffer, sizeof(GibCore::ImageFilterParams), 0, (sockaddr*)&newCli, &_slen);
	memcpy(&params, paramsBuffer, sizeof(GibCore::ImageFilterParams));
	return params;
}

/// <summary>
/// Filters images
/// </summary>
/// <param name="img">Image</param>
/// <param name="params">ImageFilterParams - contains filters and arguments</param>
/// <returns></returns>
cv::Mat UDPServer::FilterImage(cv::Mat& img, GibCore::ImageFilterParams& params)
{
	// Convert params.params into a vector of strings
	std::vector<std::string> parameters = GetVectorOfString(params);

	// Filter based on the num
	Filter* filter =  GetFilterFromEnum(params.filter);
	// Runs filter polymorphicly
	cv::Mat filteredImage = filter->RunFilter(img, parameters);
	return filteredImage;
}

/// <summary>
/// Sends image to the server
/// </summary>
/// <param name="img">Image</param>
/// <param name="ext">Image Extension</param>
/// <param name="threadSocket">Socket</param>
/// <param name="clientSocket">Client address</param>
void UDPServer::SendImage(cv::Mat& img, std::string& ext, SOCKET& threadSocket, sockaddr_in clientSocket)
{
	// Creates a vector of uchars and populates it with the image data
	std::vector<uchar> buf;
	cv::imencode(ext, img, buf);
	// Gets the vector size as the size of the image
	size_t imgSize = buf.size();
	// Copies size into a temporary char buffer
	char sizeChar[sizeof(size_t)];
	memcpy(sizeChar, &imgSize, sizeof(size_t));
	// Sends char buffer to the client
	while (sendto(threadSocket, sizeChar, sizeof(size_t), 0, (sockaddr*)&clientSocket, _slen) == SOCKET_ERROR)
	{
		std::cout << "Error sending the size back to the client" << std::endl;
	}

	// Gets the size and a pointer to the start of the data
	size_t remainingToSend = buf.size();
	uchar* from = &buf[0];
	// Loops the the entirety of the size of the data
	while (remainingToSend > 0)
	{
		// Sets the packet size based on the minimum between the remaining size and a set size
		size_t sendSize = remainingToSend > UDP_BUF_SIZE ? UDP_BUF_SIZE : remainingToSend;
		while (sendto(threadSocket, (const char*)from, sendSize, 0, (sockaddr*)&clientSocket, _slen) == SOCKET_ERROR)
		{
			printf("Error during send, retrying...\n");
		}
		// Updates the size and the pointer
		remainingToSend -= sendSize;
		from += sendSize;
		std::cout << remainingToSend << " - What is left to send" << std::endl;
	}
}

/// <summary>
/// Closes socket and cleans up winsock
/// </summary>
void UDPServer::CloseAndCleanup()
{
	closesocket(_serverSocket);
	WSACleanup();
}

/// <summary>
/// Returns a Filter object from an Enum
/// </summary>
/// <param name="filter">Filter enum</param>
/// <returns>Filter object</returns>
Filter* UDPServer::GetFilterFromEnum(GibCore::ImageFilter filter)
{
	// Switch based on the enum
	switch (filter)
	{
	case GibCore::ImageFilter::ROTATION:
		return CreateFilter<Rotate>();
	case GibCore::ImageFilter::TOGREYSCALE:
		return CreateFilter<Greyscale>();
	case GibCore::ImageFilter::FLIPPING:
		return CreateFilter<Flip>();
	case GibCore::ImageFilter::RESIZE:
		return CreateFilter<Resize>();
	case GibCore::ImageFilter::CROPPING:
		return CreateFilter<Crop>();
	case GibCore::ImageFilter::BOXBLUR:
		return CreateFilter<BoxBlur>();
	case GibCore::ImageFilter::SHARPENING:
		return CreateFilter<Sharpening>();
	case GibCore::ImageFilter::BRIGHTNESSADJUST:
		return CreateFilter<BrightnessAdjust>();
	case GibCore::ImageFilter::GAMMACORRECTION:
		return CreateFilter<GammaCorrection>();
	case GibCore::ImageFilter::CONTRASTADJUST:
		return CreateFilter<ContrastAdjust>();
	}
}
/// <summary>
/// Loops through the ports vector and removes the piece of data supplied in the function. Locks to ensure the vector is properly altered
/// </summary>
/// <param name="port">Port to Remove</param>
void UDPServer::RemovePort(int port)
{
	_mutex.lock();
	// Loops through every element
	for (int i = 0; i < _usedPorts.size(); i++)
	{
		// Checks to see if the current element is equal to the port supplied in the function
		if (_usedPorts[i] == port)
		{
			// Erase value at the current position
			_usedPorts.erase(_usedPorts.begin() + i);
			break;
		}
	}
	_mutex.unlock();
}

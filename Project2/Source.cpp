#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>

#include <winsock2.h>

#include "CSocket.h"
#include "CProxy.h"
#include "CCommunicationsManager.h"

#pragma comment(lib,"ws2_32.lib")

int main()
{
	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		std::cout << "Socket creation failed.\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	SOCKADDR_IN serverInf;
	serverInf.sin_family = AF_INET;
	serverInf.sin_addr.s_addr = INADDR_ANY;
	serverInf.sin_port = htons(1815);

	if (bind(Socket, (SOCKADDR*)(&serverInf), sizeof(serverInf)) == SOCKET_ERROR)
	{
		std::cout << "Unable to bind socket!\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	CCommunicationsManager communicationsManager;

	std::vector<std::unique_ptr<CProxy>> proxiesArray;

	listen(Socket, SOMAXCONN);

	SOCKET TempSock = SOCKET_ERROR;
	while (1)
	{
		while (TempSock == SOCKET_ERROR)
		{
			std::cout << "Waiting for incoming connections...\r\n";
			TempSock = accept(Socket, NULL, NULL);
		}

		CSocket clientSide(TempSock);
		CSocket serverSide;
		if (serverSide.Initialize() == -1)
			return -1;
		
		communicationsManager.AddCommunication(std::move(clientSide), std::move(serverSide));

// 		proxiesArray.emplace_back(std::make_unique<CProxy>(std::move(clientSide), std::move(serverSide)));
// 		//CProxy(std::move(clientSide), std::move(serverSide));
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		TempSock = SOCKET_ERROR;
	}
	

	//std::cout << "Client connected!\r\n\r\n";



	// Shutdown our socket
	shutdown(Socket, SD_SEND);

	// Close our socket entirely
	closesocket(Socket);

	// Cleanup Winsock
	WSACleanup();
	system("PAUSE");
	return 0;
}
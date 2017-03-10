#pragma once

#include <iostream>
#include <winsock2.h>

class CSocket
{
//Constructors and destructors
public:
	CSocket();

	CSocket(SOCKET sock);

	CSocket(CSocket&& rCommunicator);

	CSocket(const CSocket& copy) = delete;

	CSocket& operator= (const CSocket&) = delete;

	~CSocket();


//public methods
public:
	int Initialize(const char* hostName = "localhost", unsigned short port = 1518);

	int Send(const char* buf, int len, int flags) const;

	int Receive(char* buf, int len, int flags) const;

//private members
private:
	SOCKET m_socket;
	bool m_initialized;
};
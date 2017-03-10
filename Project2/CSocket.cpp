#include "CSocket.h"
//#pragma comment(lib,"ws2_32.lib")



CSocket::CSocket() :
m_socket(INVALID_SOCKET),
m_initialized(false)
{

}

CSocket::CSocket(SOCKET sock) :
m_socket(sock),
m_initialized(true)
{
	u_long iMode = 1;
	ioctlsocket(m_socket, FIONBIO, &iMode);
}

CSocket::CSocket(CSocket&& rCommunicator)
{
	m_socket = rCommunicator.m_socket;
	m_initialized = rCommunicator.m_initialized;
	rCommunicator.m_socket = INVALID_SOCKET;
	rCommunicator.m_initialized = false;
}

int CSocket::Initialize(const char* hostName, unsigned short port)
{
	if (m_initialized || m_socket != INVALID_SOCKET)
		return -1;

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		std::cout << "Winsock error - Socket creation Failed!\r\n";
		return -1;
	}


	struct hostent *host;
	if ((host = gethostbyname(hostName)) == NULL)
	{
		std::cout << "Failed to resolve hostname.\r\n";
		return -1;
	}

	// Setup our socket address structure
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	// Attempt to connect to server
	if (connect(m_socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		std::cout << "Failed to establish connection with server\r\n";
		return -1;
	}

	u_long iMode = 1;
	ioctlsocket(m_socket, FIONBIO, &iMode);

	m_initialized = true;

	return 1;
}

CSocket::~CSocket()
{
	if (m_initialized && m_socket != INVALID_SOCKET)
	{
		shutdown(m_socket, SD_SEND);

		// Close our socket entirely
		closesocket(m_socket);
	}
}

int CSocket::Send(const char* buf, int len, int flags) const
{
	if (m_initialized && m_socket != INVALID_SOCKET)
		return send(m_socket, buf, len, flags);

	return -1;
}

int CSocket::Receive(char* buf, int len, int flags) const
{
	if (m_initialized && m_socket != INVALID_SOCKET)
		return recv(m_socket, buf, len, flags);

	return -1;
}


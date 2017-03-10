#pragma once

#include <iostream>
#include <thread>
#include "CSocket.h"
#include <atomic>

#define BUFFER_SIZE  1024*100

class CProxy
{
public:
	CProxy() = delete;

	CProxy(const CProxy& proxy) = delete;
	
	CProxy(CSocket&& rLeft, CSocket&& rRight):
		m_leftSide(std::move(rLeft)),
		m_rightSide(std::move(rRight)),
		m_stopFlag(false)
	{
		auto threadFunc = [this](CSocket& receiver, CSocket& sender)
		{
			char buffer[BUFFER_SIZE];
			int retValue = 1;
			while (!m_stopFlag && (retValue = receiver.Receive(buffer, BUFFER_SIZE, 0)) && retValue != -1)
			{
				int sendResult = 0;
				if (!(sendResult = sender.Send(buffer, retValue, 0)) || sendResult == -1)
					return;

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
		};

		m_workerThreadLeft = std::thread(threadFunc, std::ref(m_leftSide), std::ref(m_rightSide));
		m_workerThreadRight = std::thread(threadFunc, std::ref(m_rightSide), std::ref(m_leftSide));

	}

	~CProxy()
	{
		m_stopFlag.store(true);

		m_workerThreadLeft.join();
		m_workerThreadRight.join();
	}

private:
	CSocket m_leftSide;
	CSocket m_rightSide;

	std::thread m_workerThreadLeft;
	std::thread m_workerThreadRight;

	std::atomic<bool> m_stopFlag;



};
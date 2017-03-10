#pragma once

#include <thread>
#include <memory>
#include <queue>
#include <mutex>
#include <vector>
#include <map>
#include <condition_variable>
#include <algorithm>

#include "CSocket.h"

#define JOB_AMOUNT_LIMIT 10

class CCommunicationsManager
{
public:

	CCommunicationsManager();


	void AddCommunication(CSocket left, CSocket right);

private:

	using smart_lock = std::unique_lock<std::mutex>;

	struct CommunicationUnit
	{
		CommunicationUnit(CSocket left, CSocket right);
		
		CommunicationUnit(const CommunicationUnit&) = delete;

		CommunicationUnit(CommunicationUnit&& rValue);

		char leftQueue[1024 * 200]; //it's better to alloc this mem on heap due to pushing this struct to containers
		char rightQueue[1024 * 200];

		int inSize = 0;
		int outSize = 0;
		
		CSocket leftSock;
		CSocket rightSock;
	};

	std::queue<CommunicationUnit> m_pendingCommunications;
	std::vector<std::thread> m_threadPool;
	std::map<std::thread::id, size_t> m_activeJobAmount;

	volatile bool m_awake;

	std::mutex m_communicationsLock;
	std::condition_variable m_condVar;
};
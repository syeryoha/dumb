#include "CCommunicationsManager.h"

CCommunicationsManager::CommunicationUnit::CommunicationUnit(CommunicationUnit&& rValue) :
leftSock(std::move(rValue.leftSock)),
rightSock(std::move(rValue.rightSock)),
inSize(rValue.inSize),
outSize(rValue.outSize)
{}

CCommunicationsManager::CommunicationUnit::CommunicationUnit(CSocket left, CSocket right) :
leftSock(std::move(left)),
rightSock(std::move(right)),
inSize(0),
outSize(0)
{}

CCommunicationsManager::CCommunicationsManager() :
m_awake(false)
{}

void CCommunicationsManager::AddCommunication(CSocket left, CSocket right)
{
	smart_lock l(m_communicationsLock);

	m_pendingCommunications.emplace(std::move(left), std::move(right));

	// if 0 - delete this thread

	auto itNext = std::remove_if(std::begin(m_threadPool), std::end(m_threadPool), [this](const std::thread& thr)
	{
		auto it = std::end(m_activeJobAmount);
		if ((it = m_activeJobAmount.find(thr.get_id())) != std::end(m_activeJobAmount) &&
			it->second == 0)
		{
			m_activeJobAmount.erase(it); //remove record about thread's job amount from the map
			return true;
		}

		return false;
	});

	for (auto it = itNext; it != end(m_threadPool); ++it)
		it->join();
	m_threadPool.erase(itNext, std::end(m_threadPool));


	//decide if we should create a worker thread
	size_t amountOfJobIsTaken = 0;
	for (const auto& el : m_activeJobAmount)
		amountOfJobIsTaken += el.second;

	m_pendingCommunications.emplace(std::move(left), std::move(right));

	if (amountOfJobIsTaken >= JOB_AMOUNT_LIMIT * m_threadPool.size()) //actually it can't be >   
	{
		m_threadPool.emplace_back(std::thread([this]()
		{
			std::vector<CommunicationUnit> activeCommunications;

			auto TryToGetAJob = [&]()
			{
				int tries = 3;
				while (tries--)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
					smart_lock l(m_communicationsLock);
					if (!m_pendingCommunications.empty())
					{
						activeCommunications.emplace_back(std::move(m_pendingCommunications.front()));
						m_pendingCommunications.pop();
						m_activeJobAmount[std::this_thread::get_id()]++;
					}
				}
			}; TryToGetAJob();


			char buffer[1024 * 100];

			while (!activeCommunications.empty())
			{
				for (auto& communicationUnit : activeCommunications)
				{
					int messageLength = communicationUnit.leftSock.Receive(buffer, sizeof(buffer), 0);
						
					int nError = WSAGetLastError();
					if (messageLength == -1 && nError != WSAEWOULDBLOCK && nError != 0)
					{
						//OPClient is disconnected
						int a = 0;
					}
					else if (messageLength > 0)
					{
						int bytesToSend = messageLength;
						int tries = 5;
						int bytesSent = 0;
						while (messageLength && tries)
						{
								
							int res = communicationUnit.rightSock.Send(buffer, bytesToSend, 0);
							nError = WSAGetLastError();

							if (res == -1 && nError != WSAEWOULDBLOCK && nError != 0)
							{
								//Server is disconnected 
								int a = 0;
							}
							else if (res > 0)
							{
								bytesSent += res;
								messageLength -= res;
							}
						}

						if (messageLength)
						{
							memcpy(communicationUnit.rightQueue + communicationUnit.outSize, buffer + bytesSent + 1, messageLength);
							communicationUnit.outSize += messageLength;
							messageLength = 0;
						}
					}

					//Back side
						

					messageLength = communicationUnit.rightSock.Receive(buffer, sizeof(buffer), 0);

					nError = WSAGetLastError();
					if (messageLength == -1 && nError != WSAEWOULDBLOCK && nError != 0)
					{
						//Server is disconnected
						int a = 0;
					}
					else if (messageLength > 0)
					{
						int bytesToSend = messageLength;
						int tries = 5;
						int bytesSent = 0;
						while (messageLength && tries)
						{
							int res = communicationUnit.leftSock.Send(buffer, bytesToSend, 0);
							nError = WSAGetLastError();

							if (res == -1 && nError != WSAEWOULDBLOCK && nError != 0)
							{
								//OPCLient is disconnected 
								int a = 0;
							}
							else if (res > 0)
							{
								bytesSent += res;
								messageLength -= res;
							}
						}

						if (messageLength)
						{
							memcpy(communicationUnit.leftQueue + communicationUnit.outSize, buffer + bytesSent + 1, messageLength);
							communicationUnit.outSize += messageLength;
							messageLength = 0;
						}
					}
				}

				TryToGetAJob();
			}
			
		}));
	}
	else //some of not fully busy threads will get task from the queue
	{}
}

#include "DaemonStatus.h"

#define TIME_TO_RECEIVE_DATA 10
#define CHECK_STATUS_INTERVAL 10

DaemonStatus* DaemonStatus::m_instance = nullptr;

DaemonStatus::DaemonStatus(int intervalToReceiveData, int intervalToCheck) : kSecondsToReceiveData(intervalToReceiveData), kSecondsToCheckDevices(intervalToCheck)
{

}
DaemonStatus::~DaemonStatus() {}


void DaemonStatus::startThread(DaemonThreads threadName)
{
	m_threadActive[static_cast<int>(threadName)] = true;
}

void DaemonStatus::stopThread(DaemonThreads threadName)
{
	m_threadActive[static_cast<int>(threadName)] = false;
	isThreadsRunning();
}

void DaemonStatus::dataWasReceive(DaemonThreads threadName)
{

	m_lastDataWasReceive[static_cast<int>(threadName)] = time(NULL);
}

void	DaemonStatus::isThreadsRunning()
{
	// check if running all threads 
	for (int i = 0; i < THREADS_NUMBER; ++i) {
		if (m_threadActive[i] == false) {
			syslog(LOG_EMERG, kFormatStrThreadWasStopped, kThreadNames[i]);
		}
	}
}

bool	DaemonStatus::isThreadGetsData(DaemonThreads threadName)
{
	switch (threadName) // diff time for threads? 
	{ 
	case DaemonThreads::DYNAMIC: // every 1 sec 
	case DaemonThreads::TI: // every 
	case DaemonThreads::NRF: // every 5 sec
		if (difftime(time(NULL), m_lastDataWasReceive[static_cast<int>(threadName)]) > kSecondsToReceiveData)
			return false;
		default:
			break;
	}
	return true;
}

void DaemonStatus::checkCurrentState()
{
	if (difftime(time(NULL), m_lastTimeCheck) < kSecondsToCheckDevices) return ;
	m_lastTimeCheck = time(NULL);
	syslog(LOG_DEBUG, kFormatStrStatusLog,
		isThreadGetsData(DaemonThreads::DYNAMIC) ? kStatusActive : kStatusDown,
		isThreadGetsData(DaemonThreads::TI) ? kStatusActive : kStatusDown,
		isThreadGetsData(DaemonThreads::NRF) ? kStatusActive : kStatusDown
		);
}

DaemonStatus *DaemonStatus::instance() {
    if (!m_instance) {
        m_instance = new DaemonStatus(TIME_TO_RECEIVE_DATA, CHECK_STATUS_INTERVAL);
    }

    return m_instance;
}


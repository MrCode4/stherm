#pragma once 

#include <syslog.h>
#include <time.h>
#define THREADS_NUMBER 3



#include <QObject>

/**
 * @brief enum class for define threads
 * 
 */
enum class DaemonThreads
{
	DYNAMIC = 0,
	TI,
	NRF
};

/**
 * @brief Class for control work of app. It saves last time of receive data from nrf, ti or web.
 * Check every CHECK_STATUS_INTERVAL sec saved times and logs it as LOG_DEBUG 
 * 
 */
class DaemonStatus : public QObject
{
    Q_OBJECT
public: 
/**
 * @brief Construct a new Daemon Status object. Don't use.
 * 
 */
	DaemonStatus() = delete;

    static DaemonStatus* instance();

	/**
	 * @brief Destroy the Daemon Status object
	 * 
	 */
	~DaemonStatus();
	/**    
	@brief Mark thread as started to work
	@param[in] threadName thread (enum)
	*/
	void startThread(DaemonThreads threadName);

/**    
	@brief Mark thread as stopped to work
	@param[in] threadName thread (enum)
*/
	void stopThread(DaemonThreads threadName);

/**    
	@brief Save last time of thread receive data 
	@param[in] threadName thread (enum)
*/
	void dataWasReceive(DaemonThreads threadName);

/**    
	@brief every CHECK_STATUS_INTERVAL check that thread receive data from device (TI, NRF, WEB) less then TIME_TO_RECEIVE_DATA sec ago. 
	Print in log as LOG_DEBUG status of all threads
*/
	void checkCurrentState();
	
private:
    /**
 * @brief Construct a new Daemon Status object
 *
 * @param intervalToReceiveData if data was received more then intervalToReceiveData seconds, device consider down
 * @param intervalToCheck seconds for check last data receive
 */
    DaemonStatus(int intervalToReceiveData, int intervalToCheck);

	const int	kSecondsToReceiveData;
	const int	kSecondsToCheckDevices;
	const char* kFormatStrStatusLog = "DaemonStatus: Dynamic: %s, TI: %s, NRF: %s";
	const char* kFormatStrThreadWasStopped = "%s thread is not running!";
	const char*	kStatusActive = "active";
	const char*	kStatusDown = "down";
	const char* kThreadNames[THREADS_NUMBER] = {"dynamic", "ti", "nrf"};
	bool		m_threadActive[THREADS_NUMBER] = {false,};
	time_t		m_lastDataWasReceive[THREADS_NUMBER];
	time_t		m_lastTimeCheck = 0;

/**    
	@brief  
		use in checkCurrentState method. checks time 
	@param[in] threadName thread (enum)
*/
	bool 	isThreadGetsData(DaemonThreads threadName);

/**
 * @brief use in stopThread method. checks all thread for stop bool
 * 
 */
	void	isThreadsRunning();

    static DaemonStatus *m_instance;
};
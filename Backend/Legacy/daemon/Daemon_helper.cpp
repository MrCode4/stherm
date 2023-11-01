/**
 * @file Daemon_helper.cpp
 * @brief Implementation of the functions to create a daemon process.
 */
#include "Daemon_helper.h"

/**
* @brief Callback function for handling signals.
*
* @param sig Identifier of signal
*/
static char pid_file_name[] = "/run/hvac_daemon.pid";
int pid_fd;
void handle_signal(int sig)
{

	if (sig == SIGHUP || sig == SIGTERM) {
		syslog(LOG_INFO, "Debug: killing daemon ...\n");
		/* Unlock and close lockfile */
		if (pid_fd != -1) {
			lockf(pid_fd, F_ULOCK, 0);
			close(pid_fd);
		}
		/* Try to delete lockfile */
		if (pid_file_name != nullptr) {
			unlink(pid_file_name);
		}
		/* Reset signal handling to default behavior */
		signal(SIGHUP, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		exit(0);
	}
}

/**
 * @brief This function will daemonize this app.
 */
void daemonize()
{

	pid_t pid = 0;
	int fd;
	
	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* On success: The child process becomes session leader */
	if (setsid() < 0) {
		exit(EXIT_FAILURE);
	}

	/* Ignore signal sent from child to parent process */
	signal(SIGCHLD, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/usr/share/apache2/default-site/htdocs/engine/");

	/* Close all open file descriptors */
	for (fd = static_cast<int>(sysconf(_SC_OPEN_MAX)); fd > 0; fd--) {
		close(fd);
	}

	/* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
	stdin = fopen("/dev/null", "r");
	stdout = fopen("/dev/null", "w+");
	stderr = fopen("/dev/null", "w+");
	if (pid_file_name != nullptr)
	{
		char str[256];
		pid_fd = open(pid_file_name, O_RDWR | O_CREAT, 0640);
		if (pid_fd < 0) {
			/* Can't open lockfile */
			exit(EXIT_FAILURE);
		}
		if (lockf(pid_fd, F_TLOCK, 0) < 0) {
			/* Can't lock file */
			exit(EXIT_FAILURE);
		}
		/* Get current PID */
		sprintf(str, "%d\n", getpid());
		/* Write PID to lockfile */
		write(pid_fd, str, strlen(str));
	}


	/* Daemon will handle two signals */
	signal(SIGTERM, handle_signal);
	signal(SIGHUP, handle_signal);
	
}

/**
 * @brief Parse comand line arguments to log level
 * 
 * @param[in] argc Number of command line arguments 
 * @param[in] argv Array of command line arguments
 * @param[out] log_level_val reference to log_level
 * @return true arguments valid and and set log level to argument
 * @return false arguments invalid. log level set to default value (LOG_INFO)
 */
bool parseLogLevelOpt(int argc, char* argv[], int& log_level_val)
{
    log_level_val = LOG_INFO; // just in case. 
	if (argc != 2)
		return false;

	std::string log_level_arg(argv[1]);
	if (log_level_arg.find(LOG_LEVEL_KEY) == std::string::npos)
		return false;
	std::string log_level_str = log_level_arg.substr(strlen(LOG_LEVEL_KEY));
	if (log_level_str.empty())
		return false;

// parse str val to log level val
    if (log_level_str == "error")
        log_level_val = LOG_ERR;
    else if (log_level_str == "info")
        log_level_val = LOG_INFO;
    else if (log_level_str == "all")
        log_level_val = LOG_DEBUG;
	else 
		return false;

    return true;
}

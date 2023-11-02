#pragma once
#include "Peripheral.h"

#define LOG_LEVEL_KEY "--log-level="

/**
 * @brief Callback function for handling signals.
 *
 * @param sig Identifier of signal.
 */

void handle_signal(int sig);
/**
 * @brief This function will daemonize this app.
 */
void daemonize();

/**
 * @brief Parse command line argument to log level.
 * 
 * @param argc number of command line arguments
 * @param argv array of command line arguments
 * @param[out] log_level_val val of log level
 * @return bool of status parsing 
 */
bool parseLogLevelOpt(int argc, char* argv[], int& log_level_val);


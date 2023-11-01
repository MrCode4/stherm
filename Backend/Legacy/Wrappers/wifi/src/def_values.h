/**
 * @file def_values.h
 * @brief Contains constants and default values used in the InputParametrs class for managing Wi-Fi connections.
 *
 * This file defines constants such as the minimum and maximum number of command-line arguments accepted,
 * the available Wi-Fi management commands, and default values for Wi-Fi manual connection parameters.
 * @author Gor Danielyan
 * @author Contact https://t.me/Gorynych_DuO
 */
#pragma once
#include <iostream>
#define ARGUMENT_MIN_COUNT 2 /**< The minimum number of command-line arguments required for a valid command */
#define ARGUMENT_MAX_COUNT 10 /**< The maximum number of command-line arguments accepted for a command */
std::string my_cmd_array[] = { "search","manual","connect","disconnect","forget","active","connections","secret_delete_all"}; /**< The aviable list of command that @link class InputParametrs @endlink can wotk with*/
int my_arr_size = 8;
/**
 * @brief The available Wi-Fi management commands as enumerated values.
 */
enum COMMANDS
{
	COMMAND_SEARCH=100,
	COMMAND_MANUAL,
	COMMAND_CONNECT,
	COMMAND_DISCONNECT,
	COMMAND_FORGET,
	COMMAND_SHOW_ACTIVE_CONNECTIONS,
	COMMAND_SHOW_CONNECTIONS,
	COMMAND_DELETE_ALL,
};
/**
 * @brief The default values for Wi-Fi manual connection parameters.
 */
std::string wifi_manual = "essid:LanTekIV ip:172.16.0.227 subnet:255.255.255.0 gtw:172.16.0.1 dns:8.8.8.8 dns2:4.4.4.4 pass:IdealNetworks123*";

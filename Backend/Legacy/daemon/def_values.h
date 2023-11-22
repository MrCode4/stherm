#pragma once
#include <iostream>
#define ARGUMENT_MIN_COUNT 2
#define ARGUMENT_MAX_COUNT 10
std::string my_cmd_array[] = { "search","manual","connect","disconnect","forget","active","connections","secret_delete_all"};
int my_arr_size = 8;
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
std::string wifi_manual = "essid:LanTekIV ip:172.16.0.227 subnet:255.255.255.0 gtw:172.16.0.1 dns:8.8.8.8 dns2:4.4.4.4 pass:IdealNetworks123*";
/**
 * @mainpage Wi-Fi Management Program
 *
 * @section intro Introduction
 *
 * This program allows users to manage Wi-Fi connections using the nmcli command-line tool.
 * It provides a simple command-line interface for searching, connecting, disconnecting,
 * and forgetting Wi-Fi networks, as well as managing saved connections and retrieving
 * information about the current Wi-Fi connection.
 *
 * The program consists of four classes:
 * - InputParametrs: Handles the command-line arguments and calls the appropriate Wi-Fi management method.
 * - WifiManager: Implements the Wi-Fi management methods using the nmcli tool.
 * - def_values: Contains constants and default values used in the InputParametrs class for managing Wi-Fi connections.
 * - main: The entry point for the program, which initializes an InputParametrs object and calls the executeCommand() method.
 *
 * @section usage Usage
 *
 * The program can be run from the command line with the following syntax:
 *
 *     wifi_manager <command> [options]
 *
 * Where `<command>` is one of the following:
 * - `search`: Search for available Wi-Fi networks.
 * - `manual`: Manually connect to a Wi-Fi network using specified parameters.
 * - `connect`: Connect to a Wi-Fi network.
 * - `disconnect`: Disconnect from a Wi-Fi network.
 * - `forget`: Remove a saved Wi-Fi network.
 * - `active`: Get MAC of the active Wi-Fi connection 
 * - `connections`: Get a list of saved Wi-Fi connections.
 * - `secret_delete_all`: Remove all saved Wi-Fi connections.
 *
 * The `search` command can be run with no options to get a list of available Wi-Fi networks.
 * The `connect` command requires the SSID and password of the network to connect to.
 * The `manual` command requires the following parameters: ESSID, IP address, subnet mask, default gateway, DNS server, DNS server 2, and password.
 * The `disconnect` and `forget` commands require the SSID of the network to disconnect or forget.
 *
 * @section install_sec Installation
 *
 * To build and install the project, follow these steps:
 * 1. Install the required dependencies: nmcli, g++
 * 2. Clone the project repository from GitHub.
 * 3. Navigate to the project directory.
 * 4. Run the following command to build the project:
 *
 *    g++ main.cpp InputParametrs.cpp WifiManager.cpp -o wifi
 *
 * @section exmaples Examples
 *
 * The `search` ./wifi search 
 * The `connect` ./wifi connect SSID PASSWORSD[optional]  
 * The `manual` ./wifi manual essid:LanTekIV ip:172.16.0.227 subnet:255.255.255.0 gtw:172.16.0.1 dns:8.8.8.8 dns2:4.4.4.4 pass:IdealNetworks123*
 * The `disconnect` ./wifi disconnect SSID[active] 
 * The `forget` ./wifi forget SSID
 * The `active` ./wifi active  
 * The `connections` ./wifi connections 
 * The `secret_delete_all` ./wifi secret_delete_all 
 *
 * @section dependencies Dependencies
 *
 * The program depends on the nmcli tool, which should be pre-installed on most Linux distributions.
 * Be careful with using sudo
 * 
 *
 * @section authors Authors
 *
 * - Gor Danielyan
 * - Contact: https://t.me/Gorynych_DuO
  @section links_sec Links
 *
 * - GitHub repository: https://github.com/DuO-cmd/newHvAc_wifi_search.git
 */

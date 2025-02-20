/**
 * @file WifiManager.h
 * @brief Header file for the WifiManager class.
 * @author Gor Danielyan
 * @author Contact https://t.me/Gorynych_DuO
 */
#pragma once
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#define DEBUG

#include <iostream>


 /**
  * @class WifiManager
  * @brief A class for managing WiFi connections using nmcli.
  * @details using nmcli commands for search/connect/disconnect/forget/manual connect wifi
  * 
  */
class WifiManager
{
public:
    /**
     * @brief Descritpion - Printing a list of available WiFi networks and their properties.
     * This function uses the "nmcli" command to get a list of available WiFi networks
     * and their properties. The properties include ESSID, unique name, connection status,
     * security status, and signal strength.
     * @param NO
     * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
     * @note wifi parametrs are printing in json format @link test.json @endlink
     * @note nmcli command that used "nmcli -f ALL dev wifi"
     */
    int get_wifi_list();

    /**
    * @brief Retrieves the list of available Wi-Fi networks and prints it in @link test.json @endlink format.
    * This function uses the 'nmcli' command to obtain the list of available Wi-Fi networks.
    * It then formats and prints the Wi-Fi networks as a JSON object.
    * @param name The active WiFi SSID.
    * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
    * @note This member is for debugging.
    * @warning working only if connection is active
    * @note nmcli command that used "nmcli -f ALL dev wifi"
    */
    int get_wifi_state(std::string name);

    /**
    * @brief Attempts to connect to a Wi-Fi network with the specified parameters.
    * This function attempts to connect to a Wi-Fi network with the specified name and optional password.
    * It first checks if a connection with the same name already exists, in which case it returns an error.
    * If the network requires a password and a password is not provided or the wrong number of arguments
    * are provided, it returns an error. If the connection is successful, it returns 0.
    * @param wifi_params A vector containing the name of the network to connect to and an optional password.
    * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
    * @note for connecting use " nmcli d wifi connect '" + name + "'" + " password '" + pass + "'"
     */
    int wifi_connect(std::vector <std::string>& wifi_params);
   
    /**
    * @brief Disconnects from the specified Wi-Fi network by name.
    * This function searches for the specified Wi-Fi network name and disconnects from it if it's active.
    * @param name A std::string containing the name of the Wi-Fi network to disconnect from.
    * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
    * @warning working only if connection is active
    * @warning disconnect and forget is differetn things
    * @note for disconnecting use nmcli c down + name
    * 
    */
    int wifi_disconnect(std::string name);
    
    /**
      * @brief Forget a WiFi connection from the nmcli connection list.
      * @param name The WiFi SSID.
      * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
      * @warning You may have big problem if these method have problem
      * @note nmcli connection show is not the available WiFi list, it is a list that saves your added connections.
      */
    int wifi_forget(std::string name);
    

    /**
     * @brief Helper method for connecting to a previously disconnected WiFi connection.
     * @param name The WiFi SSID that is in the nmcli connection list.
     * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
     * @note nmcli connection show is not the available WiFi list, it is a list that saves your added connections.
     */
    int wifi_con_up(std::string name);
    
    /**
    * @brief Get the currently active WiFi connection/s.
    * @param NO
    * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
    * @note Debugging information
    */
    int get_active_wifi();
    /**
    * @brief Get the list of WiFi connections.
    * @param NO
    * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
    */
    
    int get_wifi_connections();
    
    /**
     * @brief Delete all saved connections.
     * @param NO
     * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
     * @note nmcli connection show is not the available WiFi list, it is a list that saves your added connections.
     * @note this function use @link int wifi_forget(std::string name) @endlink
     */
    int delete_all_con();
    
    /**
     * @brief Convert a BSSID to an SSID.
     * @param bssid The BSSID to be converted.
     * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
     * @note bssid is referance
     * @warning used pass by reference
     */
    int bssid_to_ssid(std::string& bssid);
    /**
     * @brief Write the WiFi signal level.
     * @brief using for load currenct wifi signal level to dtabase
     * @return An integer indicating the success or failure of the function return code is here @link enum ERROR @endlink
     * @warning if .php file or destination will be changed change function
     */
    
    int write_wifi_signal();

    /**
    * @brief Manually connects to a Wi-Fi network using the provided network settings.
    *
    * This function scans for available Wi-Fi networks, finds the one matching the input SSID,
    * creates a new connection with the specified IP address, subnet, gateway, and DNS settings,
    * and then connects to it.
    *
    * @param args A std::vector of std::string containing the following network settings:
    *             - args[0]: SSID of the Wi-Fi network to connect to
    *             - args[1]: IP address for the connection
    *             - args[2]: Subnet mask for the connection
    *             - args[3]: Gateway address for the connection
    *             - args[4]: Primary DNS server for the connection
    *             - args[5]: Secondary DNS server for the connection
    *             - args[6]: Wi-Fi password for the connection
    *
    * @return Returns 0 on successful connection, ERROR_INTERNAL_ERROR if the parser function fails,
    *         ERROR_CONNECTION_ALREADY_EXISTS if a connection with the specified SSID already exists,
    *         ERROR_WRONG_ARGUMENTS if the specified SSID is not found in the available networks,
    *         or the error code returned by the system() function when executing nmcli commands.
    */
    int wifi_manual_connect(std::vector<std::string>& args);
    /**
     * @enum ERROR
     * @brief Enumeration of error codes used by the WifiManager class.
     */
    enum ERROR
    {
        ERROR_NO,// Success – indicates the operation succeeded.
        ERROR_UNKNOWN,//Unknown or unspecified error.
        ERROR_INPUT,//Invalid user input, wrong nmcli invocation.
        ERROR_TIMEOUT,//Timeout expired.
        ERROR_ConActFail,//Connection activation failed.
        ERROR_ConDactFail,// Connection deactivation failed.
        ERROR_DiscDevFail,//Disconnecting device failed.
        ERROR_ConDelFail,// Connection deletion failed.
        ERROR_NetMng,//NetworkManager is not running.
        ERROR_NotExsist=10,//Connection, device, or access point does not exist.
        ERROR_WRONG_BSSID,
        ERROR_WRONG_NAME,
        ERROR_WRONG_PASS,
        ERROR_NO_ACTIVE_WIFI,
        ERROR_WRONG_ARGUMENTS,
        ERROR_CRITICAL_DELETE,
        ERROR_INTERNAL_ERROR,
        ERROR_CONNECTION_ALREADY_EXISTS,
        ERROR_INTERNAL_ERROR_PARSING,
        ERROR_WRONG_SUBNET,
        ERROR_WRONG_IP,
        ERROR_WRONG_ESSID,
        
    };  
   /**
   * @enum Manual_connection
   * @brief Enumeration of manual connection parameter indices.
   */
    enum Manual_connection
    {
        essid,
        ip,
        subnet,
        gtw,
        dns,
        dns2,
        pass
    };

private:
       /**
       * @enum WifiSearchWord_e
       * @brief Enumeration of search words used for parsing WiFi data.
       */
    enum WifiSearchWord_e
    {
        name,
        ssid,
        ssid_hex,
        bssid,
        mode,
        chan,
        freq,
        rate,
        signal,
        bars,
        security,
        wpa_flags,
        rsn_flags,
        device,
        active,
        in_use,
        dbus_path
    };
 
    /**
    * @typedef pair
    * @brief A pair containing a string and a vector of strings.
    * @note only for insert safety
    */
    typedef std::pair<std::string, std::vector<std::string>> pair;
    /**
    * @brief Retrieves data from the output of a system command related to Wi-Fi settings and stores it in a vector of strings.
    * This method executes the given system command (set_call) using popen and reads the output line by line.
    * The output is then stored in the provided std::vector (data), with each line being a separate string.
    * If the method is successful in retrieving the data, it returns true; otherwise, it returns false.
    * @param data Reference to a std::vector of std::string objects, where the retrieved data will be stored.
    * @param set_call A const char array representing the system command to be executed.
    * @return Returns true if the data is successfully retrieved and stored in the data vector; otherwise, returns false.
     */
    bool get_data(std::vector<std::string>& data, const char set_call[]);
    /**

    * @brief Helper method to remove trailing spaces from a given string.
    * This method iterates through the input string (data) from the end to the beginning.
    * It  stops at the first non-space character encountered and trims the string accordingly.
    * The modified string is passed back by reference.
    * @note This method is a helper function and may be used internally within the WifiManager class.
    * @param data Reference to a std::string object, which will have its trailing spaces removed.
    */
    void removeSpacesFromBack(std::string& data);
    /**
     * @brief Searches for specified words in the first line of the given data and records their start and end positions.
     *
     * This function iterates through a list of words to be found in the first line of the provided data.
     * It then stores the start and end positions of each found word in separate vectors.
     *
     * @param data A reference to a std::vector of std::string containing the data in which the words will be searched.
     * @param findable_words A const std::vector of std::string containing the words to be searched in the data.
     * @param start_pos_vector A reference to a std::vector of size_t that will be populated with the start positions of the found words.
     * @param end_pos_vector A reference to a std::vector of size_t that will be populated with the end positions of the found words.
     */
    void find_words(std::vector<std::string>& data, const std::vector<std::string> findable_words, std::vector<size_t>& start_pos_vector, std::vector<size_t>& end_pos_vector);
    /**
    * @brief Parses the output of a command, extracting specified words and populating a map with the results.
    *
    * This function executes the given command, reads its output, and then searches for the specified words.
    * It constructs a map where the key is a specified element from each line, and the value is a vector containing
    * the extracted words from each line.
    *
    * @param set_call A const char array containing the command to be executed.
    * @param findable_words A std::vector of std::string containing the words to be searched in the command output.
    * @param base_map A reference to a std::map that will be populated with the extracted words from the command output.
    *                 The key is the 'key'-th element from each line, and the value is a std::vector containing
    *                 the extracted words from each line.
    * @param key An integer specifying which element from each line should be used as the key for the map.
    *
    * @return Returns true if the parsing was successful, and false otherwise.
    */
    int parser(const char set_call[], const std::vector<std::string> findable_words, std::map <std::string, std::vector<std::string>>& base_map,int key);
   
    ///  /// Redirects output to /dev/null to suppress any output or error messages.
    const std::string m_dev_nul = " > /dev/null 2>&1";
};




/**
 * @file InputParameters.h
 * @brief Provides the InputParametrs class for processing command-line arguments related to @link WifiManager.h @endlink
 * @author Gor Danielyan
 * @author Contact: https://t.me/Gorynych_DuO
 * The InputParametrs class is responsible for parsing and processing command-line arguments related to
 * Wi-Fi management tasks. It works in conjunction with the WifiManager class to handle tasks such as
 * Wi-Fi connect, disconnect, forget, manual connection, IP address validation, and subnet reformatting.
 */
#pragma once
#include <vector>
#include <string>
#include"WifiManager.h"
#include<assert.h>
 /**

 @class InputParametrs
 @brief The class for processing command-line arguments
 This class is responsible for parsing and processing command-line arguments related to Wi-Fi management tasks.
 It works in conjunction with the WifiManager class to handle tasks such as wifi connect disconnect forget manual connection,
 IP address validation, and subnet reformatting.
 */
class InputParametrs
{
public:
    /**
     * @brief InputParametrs constructor that initializes the ip_argv and ip_argc member variables.
     * This constructor takes command-line arguments as input and initializes the
     * ip_argv and ip_argc member variables, which will be used to store the
     * command-line arguments for processing by the InputParametrs class.
     * @param argv Pointer to an array of character pointers, containing the command-line arguments.
     * @param argc Integer representing the number of command-line arguments.
     */
    InputParametrs(char** argv, int argc):ip_argc(argc),ip_argv(argv)
    {
    }
    /**
    * @brief Executes the appropriate command based on the input parameters.
    * This function checks if the arguments are valid and then calls the appropriate function to execute the desired command.
    * It handles command execution for searching Wi-Fi networks, manually connecting to a network, connecting, disconnecting,
    * forgetting a network, showing active connections, showing all connections, and deleting all connections.
    * @return int 0 if the operation is successful, otherwise returns the corresponding error code.
    */
    int executeCommand();

private:
    /**
    * @brief Searches for specific words in a string and populates a vector with them.
    * @param args Vector to store found words
    * @param data String to search in
    * @param findable_words Vector of words to search for
    * @return True if all findable words were found, false otherwise
    */
    bool find_words(std::vector<std::string>& args, std::string& data, const std::vector<std::string> findable_words);
    /**

    @brief Extracts arguments from the command line input.
    @param argv Array of command line argument strings
    @param argc Number of command line arguments
    @param all_arguments Vector to store extracted arguments
    */
    bool get_arguments(char** argv, int argc, std::vector<std::string>& all_argunments);

    /**
     * @brief Determines the command type based on the command line input.
     * @param argv Array of command line argument strings
     * @param argc Number of command line arguments
     * @return Command type as an integer
     */
    int get_command(char** argv, int argc);
    /**
    * @brief Checks if the number of command-line arguments is valid for the given command.
    *
    * This function takes the command-line arguments and their count as input and
    * checks if the number of arguments is valid for the command specified by the
    * user. It returns true if the argument count is valid, otherwise returns false.
    *
    * @param argv The command-line arguments.
    * @param argc The count of command-line arguments.
    * @return Returns true if the argument count is valid for the given command, false otherwise.
    */
    bool check_arg_count(char** argv, int argc);//arguments is correct
    /**
    * @brief Prints error messages based on the given error code.
    *
    * This function takes an error code from the WifiManager::ERROR enum and
    * prints an appropriate error message to the console. If there is no error,
    * nothing will be printed.
    *
    * @param err_code An error code from the WifiManager::ERROR enum.
    * @return Returns the same error code that was passed as input.
    */
    int print_errors(int err_code);
    /**
    * @brief Validates and processes manual connection arguments.
    *
    * This function takes a vector of manual connection arguments and a reference to a string
    * containing the raw data. It checks the validity of the input arguments and sets default values
    * for missing or empty fields. If any of the input arguments are invalid, the function returns
    * an appropriate error code from the WifiManager::ERROR enum.
    *
    * @param args A vector of manual connection arguments.
    * @param data A reference to a string containing the raw data for the manual connection arguments.
    * @return Returns 0 if all the manual connection arguments are valid, otherwise returns an error code
    *         from the WifiManager::ERROR enum.
    */
    int manual_connect_arguments(std::vector<std::string>& args, std::string& data);
    /**
    * @brief Checks if the input string is a valid IP address.
    *
    * This function takes a string representing an IP address and checks
    * if it is in a valid format. A valid IP address consists of 4 octets
    * separated by dots, where each octet is an integer between 0 and 255.
    * If the input string is a valid IP address, the function returns true;
    * otherwise, it returns false.
    *
    * @param ip A string representing an IP address to be validated.
    * @return Returns true if the input string is a valid IP address, false otherwise.
    */
    bool is_ip(std::string ip); //not & because eraseing string

    /**
    * @brief Reformats a subnet mask string into CIDR notation.
    *
    * This function takes a reference to a subnet mask string and
    * reformats it into CIDR notation (e.g., "255.255.255.0" becomes "24").
    * If the provided subnet mask string is invalid, it clears the input
    * string and returns false. Otherwise, the function returns true
    * after updating the input string with the CIDR notation.
    *
    * @param subnet A reference to the subnet mask string to be reformatted.
    * @return Returns true if the input subnet mask string was successfully
    *         reformatted into CIDR notation, false otherwise.
    */
    bool subet_reformating(std::string& subnet);
    /**
     * @brief Removes trailing spaces from a string.
     *
     * This function takes a reference to a string and removes all
     * trailing spaces from it, modifying the original string.
     * @note This is a helper function used in the @link InputParametrs::find_words @endlink method.
     *
     * @param data A reference to the string from which trailing spaces will be removed.
     */
    void removeSpacesFromBack(std::string& data);

    std::vector<std::string> ip_arguments;///< Vector to store IP-related command-line arguments.

    char** ip_argv;///< Pointer to the array of command-line arguments.

    int ip_argc;///< The number of command-line arguments.

    WifiManager wifi_function; ///< Instance of WifiManager class for handling Wi-Fi connections.
    /**
   * @enum Manual_connection
   *
   * @brief Enum for indexing the wifi-IP-related command-line arguments.
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
    /**
     * @enum Subnet
     *
     * @brief Enum for representing valid subnet mask values.
     */
    enum Subnet
    {
        SUBNET_0,
        SUBNET_128,
        SUBNET_192,
        SUBNET_224,
        SUBNET_240,
        SUBNET_248,
        SUBNET_252,
        SUBNET_254,
        SUBNET_255

    };
};



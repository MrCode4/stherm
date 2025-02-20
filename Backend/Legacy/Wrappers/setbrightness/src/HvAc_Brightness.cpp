/**
@file HvAc_Brigtness_Set.cpp
@brief This file contains the main function for setting the brightness level of the HVAC system.
Program execution begins and ends with the main function. It sets the brightness level of the HVAC system
by reading the input argument provided and updating the system configuration accordingly.
@note brightness maximum level is 254 for now 
*/

#include <iostream>
#include <stdlib.h>
#include <string>
#include <iostream>
#include<fstream>
using namespace std;
/**
@brief Enumerated type representing various error codes used throughout the program.
*/
typedef enum SIO_Errors {
    ERROR_NO = 0x00, ///< No error
    ERROR_01, ///< Error: I2C_BUS
    ERROR_02, ///< Error: Temperature/Humidity is not updated
    ERROR_GPIO_INIT, ///< Error: GPIO initialization
    ERROR_UNKNOWN_COMMAND, ///< Error: Unknown command received
    ERROR_CRC, ///< Error: CRC mismatch
    ERROR_RELAY_NOT_FOUND, ///< Error: Relay not found
    USAGE_ERROR, ///< Error: Program call argument count error
    ARGUMENT_ERROR, ///< Error: Program call argument expression error
    RESOURCE_BUSY_ERROR, ///< Error: Cannot open resource associated with the program
    INTERNAL_ERROR, ///< Error: Internal error on the side of microcontrollers
    WRONG_ESSID, ///< Error: Wrong ESSID provided
    WRONG_PASSWORD ///< Error: Wrong password provided
} SIO_Errors_t;

/**
* @brief Main function for setting the brightness level of the HVAC system.
* @param argc The number of command-line arguments.
* @param argv An array of command-line arguments.
* @return int Returns 0 on successful execution or non-zero error code on failure.
*/
int main(int argc, char** argv)
{
    // Validate command-line arguments count
    if (argc != 2)
    {
        cout << USAGE_ERROR << endl;
        return 1;
    }
    // Read the input argument and create a string for brightness level
    string brigtness_level;
    for (int i = 0; argv[1][i] != '\0'; i++)
    {
        brigtness_level.push_back(argv[1][i]);
    }
    // Convert the brightness level string to integer
    int i = std::stoi(brigtness_level);
    // Check if the input argument is a valid brightness level
    if (i < 0 || i>100)
    {
        cout << ARGUMENT_ERROR << endl;
    }
    else
    {
        // Ensure the minimum brightness level is 1
        if (i == 0)
        {
            // Convert the brightness level to a corresponding value
            i = 1;
        }
        i = i * 2.54;
        string cmmd = "echo " + to_string(i) + " > /sys/class/backlight/backlight_display/brightness";
        // Execute the command to set the brightness level
        int err_code = system(cmmd.c_str());
        if (!err_code)
        {
            cout << ERROR_NO << std::endl;
            return 0;
        }
        cout << INTERNAL_ERROR;
    }
    return 0;
}
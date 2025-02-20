/**
 * @file HvAc_Timezone_Set.cpp
 * @brief This file contains the main function for setting the timezone in the HVAC system.
 *
 * The program sets the timezone for the HVAC system by reading the input argument provided
 * and updating the system configuration accordingly.
 */
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>

using namespace std;
/**
 * @brief Enumerated type representing various error codes used throughout the program.
 */
typedef enum SIO_Errors {
    ERROR_NO = 0x00,              ///< No error
    ERROR_01,                     ///< Error: I2C_BUS
    ERROR_02,                     ///< Error: Temperature/Humidity is not updated
    ERROR_GPIO_INIT,              ///< Error: GPIO initialization
    ERROR_UNKNOWN_COMMAND,        ///< Error: Unknown command received
    ERROR_CRC,                    ///< Error: CRC mismatch
    ERROR_RELAY_NOT_FOUND,        ///< Error: Relay not found
    USAGE_ERROR,                  ///< Error: Program call argument count error
    ARGUMENT_ERROR,               ///< Error: Program call argument expression error
    RESOURCE_BUSY_ERROR,          ///< Error: Cannot open resource associated with the program
    INTERNAL_ERROR,               ///< Error: Internal error on the side of microcontrollers
    WRONG_ESSID,                  ///< Error: Wrong ESSID provided
    WRONG_PASSWORD                ///< Error: Wrong password provided

} SIO_Errors_t;

string my_timezone;
/**
 * @brief Main function for setting the timezone of the HVAC system.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 * @return int Returns 0 on successful execution or non-zero error code on failure.
 */
int main(int argc, char** argv)
{
    // Validate command-line arguments count
    if (argc !=2)
    {
        cout << USAGE_ERROR << endl;
        return 0;
    }
    // Read the input argument and create a string for timezone
    for (int i = 0; argv[1][i] != '\0'; i++)
    {
        my_timezone.push_back(argv[1][i]);
    }
    // Convert the timezone string to integer
    int timezone_int = atoi(my_timezone.c_str());
    // Check if the input argument is a valid timezone value
    if (timezone_int > -12 && timezone_int < 14)
    {
        timezone_int = - timezone_int;
        if (timezone_int >= 0)
        {
            my_timezone = "+" + to_string(timezone_int);
        }
        else
        {
            my_timezone = to_string(timezone_int);
        }
        // Update the system configuration with the new timezone
        string cmmd = "sudo ln -fs /usr/share/zoneinfo/Etc/GMT" + my_timezone + " /etc/localtime";

        int err_code=system(cmmd.c_str());
        if (!err_code)
        {
            cout << ERROR_NO << endl;
            return 0;
        }
    }
    // Invalid timezone value, return error code
    cout << ARGUMENT_ERROR << endl;
    return 0;
    
}


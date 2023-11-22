/**
 * @file HvAc_Timezone_Set.cpp
 * @brief This file contains the main function for setting the timezone in the HVAC system.
 *
 * The program sets the timezone for the HVAC system by reading the input argument provided
 * and updating the system configuration accordingly.
 */

/**
 * @mainpage HVAC Timezone Configuration
 * 
 * @section intro_sec Introduction
 * This program allows you to set the timezone for an HVAC (Heating, Ventilation, and Air Conditioning) system.
 * By providing a valid timezone offset as a command-line argument, the program updates the system configuration
 * to reflect the specified timezone.
 * 
 * @section usage_sec Usage
 * To use this program, provide a valid timezone offset as a command-line argument when executing the program.
 * The program will then adjust the system configuration to use the specified timezone offset.
 * - Compile
 * @code
 *  g++ HvAcTimezone.cpp -o setTimezone
 * @endcode 
 * - Run the compiled executable.
 * @code
 *  ./setTimezone 10
 * @endcode 
 * The valid timezone offset range is from -12 to 13, representing different time zones.
 * 
 * @section error_codes_sec Error Codes
 * The program uses a set of error codes to indicate different types of errors that may occur during execution.
 * These error codes are defined in the SIO_Errors enumeration in the source code.
 * 
 * @section author_sec Author
 * This program was developed by Gor Danielyan.
 * 
 */
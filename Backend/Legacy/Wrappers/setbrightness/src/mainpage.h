/**
 * @file HvAc_Brigtness_Set.cpp
 * @brief This file contains the main function for setting the brightness level of the HVAC system.
 * 
 * Program execution begins and ends with the main function. It sets the brightness level of the HVAC system
 * by reading the input argument provided and updating the system configuration accordingly.
 *
 * @note The maximum brightness level is 254.
 */


/**
 * @mainpage HVAC Brightness Control
 * 
 * @section intro_sec Introduction
 * This program allows you to set the brightness level of an HVAC system.
 * It reads the input brightness level provided as a command-line argument and adjusts the system configuration accordingly.
 * The program ensures that the minimum brightness level is 1 and the maximum brightness level is 254.
 * 
 * @section usage_sec Usage
 * - Compile
 * @code
 *  g++ HvAc_Brightness.cpp -o setBrightness
 * @endcode 
 * - Run the compiled executable.
 * @code
 * ./setBrightness 50 
 * @endcode 
 * To use this program, provide the desired brightness level as a command-line argument when executing the program.
 * The program will then set the brightness level of the HVAC system accordingly.
 * 
 * @section error_codes_sec Error Codes
 * The program uses a set of error codes to indicate different types of errors that may occur during execution.
 * These error codes are defined in the SIO_Errors enumeration in the source code.
 * 
 * @section author_sec Author
 * This program was developed by Gor Danielyan.
 * 
 */
/**
 * @mainpage GPIO Pin Configuration and Reading
 *
 * @section intro_sec Introduction
 *
 * This documentation provides an overview of the "GPIO Pin Configuration and Reading" program. The program is designed to configure a GPIO pin as an input, read its value, and print the inverted value to the console.
 *
 * This program reads the status of a GPIO pin on a Linux-based system to determine whether the device is connected to the power board or 
 * is floating. If the GPIO pin is high (up), it indicates that the device is on the power board and is in working mode. 
 * If the GPIO pin is low (down), it indicates that the device is floating and is in testing mode. 
 *
 * @section usage_sec Usage
 *
 * To use the program, follow these steps:
 * 1. Compile 
  * @code
 *  g++ Test_mode.cpp -o getStartMode
 * @endcode
 * 2. Run the executable to configure and read the specified GPIO pin (GPIO 90 in this case).
 * @code
 * ./getStartMode
 * @endcode
 * 3. The program will configure the GPIO pin as an input, read its value, and print the inverted value (1 as 0, and 0 as 1) to the console.
 * 4. The program will handle any errors during the GPIO configuration and reading processes.
 *
 * @section dependencies_sec Dependencies
 *
 * The program depends on several standard C and C++ libraries for file I/O, string manipulation, and system calls. Additionally, it relies on the sysfs interface for interacting with GPIO pins on a Linux system.
 *
 * @section note Note
 * 
 * Please ensure that the designated pin is properly configured in the Linux Device Tree at the Kernel Level.
 * @section author_sec Author
 *
 * This program was authored by Gor Danielyan.
 */
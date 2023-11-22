/**
 * @mainpage CPU Serial Number Extractor
 *
 * @section intro Introduction
 * This program is designed to extract the serial number of the CPU from the /proc/cpuinfo file.
 * The extracted serial number is then printed to the console.
 * This program is for the web to get the serial number, they use it later.
 * @section usage Usage
 * - Compile 
   * @code
 *  g++ main.cpp -o GetUid.o
 * @endcode
 * - Run the compiled executable.
  @code
 *  ./GetUid.o
 * @endcode
 * - The serial number of the CPU will be printed to the console.
 *
 * @section details Implementation Details
 * The program reads the /proc/cpuinfo file line by line and searches for the line containing "Serial."
 * If found, it extracts the serial number and prints it to the console.
 * If not found, it prints -1 to indicate that the serial number was not found.
 *
 * @section note Note
 * This program assumes that the CPU's serial number is listed in the /proc/cpuinfo file under the "Serial" entry.
 * If the format of the file changes or the information is located differently, this program may not work as expected.
 *
 * @section author Author
 * Gor Danielyan
 *
 */
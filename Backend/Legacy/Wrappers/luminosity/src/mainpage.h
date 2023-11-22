/**
 * @file Luminosity_report.h
 * @brief This file contains the main function responsible for acquiring the Luminosity value from a TOF sensor.
 *
 * The program sets up UART communication with an nRF device and sends a command to read TOF (Time of Flight) data
 * from the connected sensor. The program retrieves Luminosity and Range (in millimeters) values, and then outputs
 * the Luminosity value.
 */

#include "Peripheral.h"

/**
 * @mainpage Luminosity Report from TOF Sensor
 *
 * @section intro_sec Introduction
 *
 *This program is designed to extract the Luminosity value from a Time of Flight (TOF) sensor via UART communication with an nRF device. It provides a straightforward way to read and report the Luminosity value from the connected TOF sensor.
 *
 * @section usage_sec Usage
 *
 * To use the program, follow these steps:
 * 1. Compile and build the program with appropriate dependencies, including the "Peripheral.h" header file.
 * @code
 *  g++ crc.c Luminosity_report.cpp  Peripheral.cpp serial_drv.c -o Luminosity.out
 * @endcode
 * 2. Run the executable to establish UART communication with the nRF device and retrieve Luminosity data.
 * @code
 *  ./Luminosity.out
 * @endcode
 * 3. The program will send a command to read TOF data from the sensor and wait for the response.
 * 4. Upon receiving valid data, the program will extract the Luminosity value and output it to the console.
 *
 * @section dependencies_sec Dependencies
 *
 * The program depends on the "Peripheral.h" header file for peripheral configurations and functions related to UART communication and packet handling.
 *
 * @section note_sec Note
 *
 * This program assumes proper hardware connections and configurations for UART communication with the nRF device and the connected TOF sensor. It is important to ensure accurate UART settings and data processing for correct results.
 *
 * @section author_sec Author
 *
 * This program was authored by Gor Danielyan.
 */
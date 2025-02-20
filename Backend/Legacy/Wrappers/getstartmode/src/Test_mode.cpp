/**
 * @file Test_mode.cpp
 * @brief This file contains the main function for configuring and reading GPIO pins.
 *
 * The program configures the GPIO pin as an input, reads its value, and prints the
 * value to the console.
 */
#define _POSIX_SOURCE
#include <sys/ioctl.h>			//Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <fcntl.h>				//Needed for SPI port
#include <sys/stat.h>//mkfifo
#include <sys/types.h>
#include <unistd.h>			//Needed for SPI port
#include <fstream>
#include <iostream>
#define EXPORT_PATH      "/sys/class/gpio/export\0"
#define SW_VAL_PATH      "/sys/class/gpio/gpio%d/value\0"
#define SW_INT_PATH      "/sys/class/gpio/gpio%d/edge\0"
#define SW_DIR_PATH      "/sys/class/gpio/gpio%d/direction\0"

 /**
  * @brief Configure the specified GPIO pin.
  *
  * @param gpio The GPIO pin to be configured.
  * @return int Returns EXIT_SUCCESS if the pin is successfully configured, EXIT_FAILURE otherwise.
  */
int configure_pins(int gpio)
{
    int fd_export, fd_edge, fd_input;
    char str[256];
    sprintf(str, "%d\0", gpio);
    /*******************EXPORT*******************/
    // open export file
    if ((fd_export = open(EXPORT_PATH, O_WRONLY)) <= 0) {
        return EXIT_FAILURE;
    }
    // export SW GPIO
    if (write(fd_export, str, strlen(str)) < 0) {
        if (errno != EBUSY) { // does not end if pin is already exported
            close(fd_export);
            return EXIT_FAILURE;
        }
    }
    // close export file
    close(fd_export);
    /******************DIRECTION******************/
    // open direction file
    sprintf(str, SW_DIR_PATH, gpio);
    if ((fd_input = open(str, O_WRONLY)) <= 0) {
        return EXIT_FAILURE;
    }
    if (write(fd_input, "in", 2) < 0) { // configure as input
        if (errno != EBUSY) {
            close(fd_input);
            return EXIT_FAILURE;
        }
    }

    close(fd_input); // close direction file
    return EXIT_SUCCESS;
}
/**
 * @brief Main function for configuring and reading GPIO pins.
 *
 * @return int Returns 0 on successful execution, non-zero error code on failure.
 */
int main()
{
    // Configure the GPIO pin
    if (configure_pins(90))
    {
        std::cout << -1<<std::endl;
        return 0;
    }
    // Read the GPIO value
    char str[36];
    sprintf(str, SW_VAL_PATH, 90);
    int fd = open(str, O_RDONLY);
    char gpio_val;
    read(fd, &gpio_val, 1);
    // Convert and print the GPIO value
    int print_val = (gpio_val-'0') ? 0 : 1;
    std::cout << print_val << std::endl;

    

}

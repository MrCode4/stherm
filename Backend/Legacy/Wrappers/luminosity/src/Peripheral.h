/**
 * @file Peripheral.h
 * @brief Contains enums, structures, and function prototypes required for communication between threads and peripherals.
 *
 * This file includes necessary header files, defines required macros, and declares structures and function prototypes
 * for handling the communication between threads and peripherals. It is mainly used for managing various alerts and
 * the configuration of GPIO pins and serial ports.
 */
#pragma once
#define _POSIX_SOURCE
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <errno.h> 
#include <termios.h>
#include <fcntl.h>				
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>			
#include <poll.h>
#include <syslog.h>
#include <signal.h>
#include "serial_drv.h"
#include "crc.h"
#include <fstream>
#include <algorithm>
#include <linux/uinput.h>
#define NRF_SERRIAL_PORT "/dev/ttymxc1"
#define TI_SERRIAL_PORT "/dev/ttymxc3"
#define NRF_GPIO_4		21
#define NRF_GPIO_5		22
#define EXPORT_PATH      "/sys/class/gpio/export\0"
#define SW_VAL_PATH      "/sys/class/gpio/gpio%d/value\0"
#define SW_INT_PATH      "/sys/class/gpio/gpio%d/edge\0"
#define SW_DIR_PATH      "/sys/class/gpio/gpio%d/direction\0"
 //#define SYS_KEY_EVNT      "xdotool key %c\0"
#define SYS_KEY_EVNT      "/usr/share/apache2/default-site/htdocs/engine/ydotool type '%c'\0"
#define VERSION_FILE      "/usr/share/apache2/default-site/htdocs/configuration/version.ini"
#define VERSION_HEAD      "[version]"
#define VERSION_SW        "SOFTWARE_VERSION ="
#define VERSION_HW        "HARDWARE_VERSION ="
#define IPLEN 512
#define SQUARE(X) ((X)*(X))
#define UNUSED_VARIABLE(X)  ((void)(X))
#define POLL_GPIO POLLPRI | POLLERR 
#define POLLERRVAL (POLLERR | POLLHUP | POLLNVAL)
/**
 * @brief Enumeration for inter-thread communication commands.
 */
enum InThreadCMDS {
    Set_limits = 0x00,
    Set_time,
    Set_paired,
    Unpair,
    Set_ignore,
    Get_wiring,
    Wiring_check,
    Set_relays,
    NO_CMD = 0xFF
};
/**
 * @brief Enumeration for LED behavior.
 */
enum Led_effect_e
{
    LED_STABLE = 0,
    LED_FADE,
    LED_BLINK,
    LED_NO_MODE
};
/**
 * @brief Enumeration for alert types for the web interface.
 */
enum Alert_types_en
{
    Alert_temp_high = 1,// +127 max
    Alert_temp_low, // -128 low
    Alert_Tvoc_high, // 255 max (tvoc value range 0.1 to 10+ mg/m^3 value is divided by 10.0)
    Alert_etoh_high, //up to 20ppm
    Alert_iaq_high, //1 to 5
    Alert_humidity_high,// up to 100%
    Alert_humidity_low,//as low as 0%
    Alert_pressure_high, //up to 1200 hPa
    Alert_c02_high,//400 to 5000ppm
    Alert_wiring_not_connected,
    Alert_could_not_set_relay,
    NO_ALlert
};
/**
 * @brief Enumeration for alert levels for the web interface.
 */
enum Alert_lvl_en
{
    LVL_Emergency = 1,
    LVL_Warning,
    LVL_UNIMPORTANT
};
#define RELAY_OUT_CNT 10
#define WIRING_IN_CNT 10
#define MAX_PAIED_CNT 16
/**
 * @brief Structure for thread data.
 */
struct thread_Data {
    int pipe_out;
    int pipe_in;
};
/**
 * @brief Function to set the TTY configuration.
 * @param tty Pointer to a termios structure to store the configuration.
 */
void set_tty(termios* tty);

/**
 * @brief Function to configure a GPIO pin as input for interrupt handling.
 * @param gpio Integer representing the GPIO pin number.
 * @return EXIT_SUCCESS on successful system call, EXIT_FAILURE otherwise.
 */
int configure_pins(int gpio);
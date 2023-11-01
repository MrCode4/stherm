#include "Peripheral.h"

/**
* @brief Configure the termios struct for the tty device.
* This function sets the options in the termios struct for the tty device, such as the data size, parity,
* and stop bits. It also sets flags to disable hardware flow control and enable READ and ignore ctrl lines.
* @param tty Pointer to the termios struct to be configured.
*/

void set_tty(termios* tty)
{
    (*tty).c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    (*tty).c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    (*tty).c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    (*tty).c_cflag |= CS8; // 8 bits per byte (most common)
    (*tty).c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    (*tty).c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    (*tty).c_lflag &= ~ICANON;
    (*tty).c_lflag &= ~ECHO; // Disable echo
    (*tty).c_lflag &= ~ECHOE; // Disable erasure
    (*tty).c_lflag &= ~ECHONL; // Disable new-line echo
    (*tty).c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    (*tty).c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    (*tty).c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes
    (*tty).c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    (*tty).c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    (*tty).c_cc[VTIME] = 0;
    (*tty).c_cc[VMIN] = 4;
    return;
}
/**
* @brief Configure the GPIO pins.
* This function exports the specified SW GPIO, configures it as input, and sets the edge detection to
* falling.
* @param gpio The GPIO pin number to be configured.
* @return Returns EXIT_SUCCESS if successful, or EXIT_FAILURE if an error occurred.
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


    /********************EDGE*********************/
    sprintf(str, SW_INT_PATH, gpio);
    while ((fd_edge = open(str, O_RDWR)) <= 0);
    while (write(fd_edge, "falling", 7) < 0);
    close(fd_edge);
    return EXIT_SUCCESS;
}

#pragma once

#include <QString>

/*! ***********************************************************************************************
 * The UtilityHelper class is a container that encapsulates
 *  a collection of static methods providing a range of general-purpose functionalities.
 *  It acts as a helper, offering a diverse set of operations that are commonly
 *  needed across various components of application.
 * ************************************************************************************************/

class UtilityHelper
{
public:

    //! This function exports the specified SW GPIO, configures it as input, and sets the edge detection to
    //! falling.
    //! gpio The GPIO pin number to be configured.
    //! return Returns true if successful
    //!  or false if an error occurred.
    static bool configurePins(int gpio);

    //! Open direction file and set pin
    static void exportGPIOPin(int pinNumber);

    static int getStartMode(int pinNumber);

    //! Get CPU info
    static QString getCPUInfo();

    //! setBrightness, value is a number between 0 and 254
    static void setBrightness(int value);

    //! Set time zone
    static void setTimeZone(int offset);
};


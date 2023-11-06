#pragma once

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

};


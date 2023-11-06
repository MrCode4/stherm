#pragma once

#include <QThread>

class DataParser : QThread
{
public:
    DataParser(QObject *parent = nullptr);

    //! This function exports the specified SW GPIO, configures it as input, and sets the edge detection to
    //! falling.
    //! gpio The GPIO pin number to be configured.
    //! return Returns true if successful
    //!  or false if an error occurred.
    bool configure_pins(int gpio);
};


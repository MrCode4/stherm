#pragma once

#include <QThread>

#include "UARTConnection.h"

/*! ***********************************************************************************************
 * This class oversees the management of the TI and UART threads and
 * handles the processing of associated signals.
 * ************************************************************************************************/

class DataParser : QObject
{
public:
    DataParser(QObject *parent = nullptr);

    //! This function exports the specified SW GPIO, configures it as input, and sets the edge detection to
    //! falling.
    //! gpio The GPIO pin number to be configured.
    //! return Returns true if successful
    //!  or false if an error occurred.
    bool configurePins(int gpio);

signals:
    void dataReay(QVariantMap data);

private:
    //! Create NRF connection
    void createNRF();

private:
        UARTConnection * uartConnection;

};


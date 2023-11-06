#pragma once

#include <QThread>

#include "UARTConnection.h"

/*! ***********************************************************************************************
 * This class oversees the management of the TI and UART threads and
 * handles the processing of associated signals.
 * ************************************************************************************************/

class DataParser : public QObject
{
    Q_OBJECT

public:
    DataParser(QObject *parent = nullptr);

signals:
    void dataReay(QVariantMap data);

private:
    //! Create NRF connection
    void createNRF();

private:
        UARTConnection * uartConnection;

};


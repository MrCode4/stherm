#pragma once

#include <QThread>

#include "UARTConnection.h"

/*! ***********************************************************************************************
 * This class oversees the management of the TI and UART threads and
 *  handles the processing of associated signals
 * (Convert serialized data into its original data types and
 *  structure it for utilization within the user interface.).
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

    //! Deserialize main data and send dataReay signal
    QVariantMap deserializeMainData(const QByteArray &serializeData);

private:
        UARTConnection * uartConnection;

};


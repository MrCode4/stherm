#pragma once

#include <QVariant>
#include <QSerialPort>
#include <QMutex>
#include <QWaitCondition>

#include "UtilityHelper.h"
/*! ***********************************************************************************************
 * Propagte a connection with UART port and read data
 * ************************************************************************************************/

class UARTConnection : public QObject
{
    Q_OBJECT

public:
    /* public Constructor
     * ****************************************************************************************/
    //! Configure and initalize UART connection
    explicit UARTConnection(const QString &portName,
                            const qint32 &baundRate,
                            QObject *parent = nullptr);

    /* public functions
     * ****************************************************************************************/
    QByteArray sendCommand(QByteArray command);

    //! Connect to device via serial port
    //! return true connection success
    //! return false connection failure
    bool startConnection();

    //! Disconnect from device
    //! return false device still connected
    //! return true device disconnected
    bool disconnectDevice();

    //! Check if the serial port is still connected
    //! return true connected
    //! return false disconnected
    bool isConnected();

    //! Send request to uart
    bool sendRequest(QByteArray data);
    bool sendRequest(const char *data, qint64 len);
    bool sendRequest(const STHERM::SIOCommand &cmd, const STHERM::PacketType &packetType);

    bool seek(qint64 pos = 0);

signals:
    void connectionError(const QString &error);

    //! Transform the data into a meaningful format
    //! and then transmit it to the intended destination.
    void sendData(QByteArray data);

private slots:

    //! Call when new data is available for reading from the device's current read channel.
    void onReadyRead();

    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort *mSerial;

    QMutex m_mutex;
    QWaitCondition m_cond;
};


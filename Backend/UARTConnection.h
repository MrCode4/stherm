#pragma once

#include <QVariant>
#include <QSerialPort>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/*! ***********************************************************************************************
 * Propagte a connection with UART port and read data
 * ************************************************************************************************/

class UARTConnection : public QThread
{
    Q_OBJECT

public:
    /* public Constractor
     * ****************************************************************************************/

    explicit UARTConnection(QObject *parent = nullptr);

    /* public functions
     * ****************************************************************************************/

    QByteArray sendCommand(QByteArray command);

    //! Configure and initalize UART connection
    void initConnection(const QString &portName, const qint32 &baundRate);


    //! Connect to device via serial port
    //! return true connection success
    //! return false connection failure
    bool connect();

    //! Disconnect from device
    //! return false device still connected
    //! return true device disconnected
    bool disconnect();


    //! Check if the serial port is still connected
    //! return true connected
    //! return false disconnected
    bool isConnected();

    //! Write Data
    bool writeData(QByteArray data);

signals:
    void connectionError(const QString &error);

    //! Transform the data into a meaningful format
    //! and then transmit it to the intended destination.
    void sendData(const QString &data);

private slots:

    //! Call when new data is available for reading from the device's current read channel.
    void onReadyRead();


private:
    void run() override;

private:

    QSerialPort *mSerial;

    QMutex m_mutex;
    QWaitCondition m_cond;
};


#pragma once

#include <QObject>

#include "UARTConnection.h"

/*! ***********************************************************************************************
 * This class manages read and write from device using UART
 * ************************************************************************************************/

class DeviceIOController : public QObject
{
    Q_OBJECT
public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    explicit DeviceIOController(QObject *parent = nullptr);
    ~DeviceIOController() = default;

    //! Send requests
    //! transfer data with UARTConnection instance
    Q_INVOKABLE QVariantMap sendRequest(QString className, QString method, QVariantList data);

    //! Set gpio
    void exportGPIOPin(int pinNumber);

    //! Get start mode
    Q_INVOKABLE int getStartMode(int pinNumber);

    //! CPU information
    Q_INVOKABLE QString getCPUInfo();

    //! Set Brightness
    Q_INVOKABLE void setBrightness(int value);

    //! Set time zone
    Q_INVOKABLE void setTimeZone(int offset);

    //! Create connections
    Q_INVOKABLE void createConnections();

    //! Create new sensor
    Q_INVOKABLE void createSensor(QString name, QString id);

signals:
    void dataReady(QVariantMap data);

    //! Send responce with requestType
    void responseReady(int requestType, QVariant response);

private:
    //! Worker thread
    QThread mThread;

    UARTConnection *uartConnection;
    UARTConnection *tiConnection;

    //! Create TI connection
    void createTIConnection();

    //! Create NRF connection
    void createNRF();
};

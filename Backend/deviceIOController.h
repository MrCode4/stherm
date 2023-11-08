#pragma once

#include <QObject>

#include "UARTConnection.h"

/*! ***********************************************************************************************
 * This class manages read and write from device using UART
 * ************************************************************************************************/

class DeviceIOController : public QThread
{
    Q_OBJECT
public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    explicit DeviceIOController(QObject *parent = nullptr);
    ~DeviceIOController() = default;

    //! Send requests
    //! transfer data with UARTConnection instance
    bool sendRequest(QString className, QString method, QVariantList data);

    //! Set gpio
    void exportGPIOPin(int pinNumber);

    //! Get start mode
    int getStartMode(int pinNumber);

    //! CPU information
    QString getCPUInfo();

    //! Set Brightness
    bool setBrightness(int value);

    //! Set time zone
    void setTimeZone(int offset);

    //! Create connections
    void createConnections();

    //! Create new sensor
    void createSensor(QString name, QString id);

signals:
    void dataReady(QVariantMap data);

    //! Send responce with requestType
    void responseReady(int requestType, QVariant response);

private:
    void run() override;

private:

    UARTConnection *uartConnection;
    UARTConnection *tiConnection;

    //! Create TI connection
    void createTIConnection();

    //! Create NRF connection
    void createNRF();
};

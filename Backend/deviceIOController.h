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
    ~DeviceIOController();

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

    bool setVacation(const int &minTemp, const int &maxTemp,
                     const int &minHumidity, const int &maxHumidity);

    //! Create connections
    void createConnections();

    //! Create new sensor
    void createSensor(QString name, QString id);

    //! Stop reading data from device
    void setStopReading(bool stopReading);

signals:
    void dataReady(QVariantMap data);

    //! Send responce with requestType
    void responseReady(int requestType, QVariant response);

private:
    void run() override;

    //! Create TI connection
    void createTIConnection();

    //! Create NRF connection
    void createNRF();

private:

    UARTConnection *uartConnection;
    UARTConnection *tiConnection;

    bool mStopReading;

};

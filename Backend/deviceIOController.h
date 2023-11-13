#pragma once

#include <QThread>

#include "DataParser.h"
#include "UARTConnection.h"
#include "UtilityHelper.h"

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
    QVariantMap sendRequest(QString className, QString method, QVariantList data);

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

    //! Update paired sensors in TI
    void updateTiDevices();

signals:
    void dataReady(QVariantMap data);

    //! Send response with requestType
    void responseReady(int requestType, QVariant response);

private:
    void run() override;

    //! Create TI connection, called each 10 seconds, getInfo (mainData,temp, hum, aq, pressure), manage requests, wiring check
    void createTIConnection();

    //! Create NRF connection, called each 30 seconds, tof, sensors, getInfo (?)
    void createNRF();

    //! Configure NRF
    void nrfConfiguration();

private:
    DataParser mDataParser;

    UARTConnection *nRfConnection;
    UARTConnection *tiConnection;

    bool mStopReading;

    //! Paired devices
    QList<STHERM::DeviceType> mDevices;
    STHERM::DeviceType mMainDevice;

};

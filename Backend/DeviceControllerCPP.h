#pragma once

#include <QQmlEngine>
#include <QString>
#include <QtNetwork>

#include "UARTConnection.h"

/*! ***********************************************************************************************
 * This class manages send requests from app to device and and process the received response.
 * ************************************************************************************************/
class DeviceControllerCPP  : public QObject
{
    Q_OBJECT

    QML_ELEMENT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    DeviceControllerCPP(QObject *parent = nullptr);
    ~DeviceControllerCPP();

    //! Exposing RequestType Enum To QML
    //! RequestType
    enum RequestType {
        Humidity = 0,
        Temperature,
        CO2,
        Fan,
        Backlight,
        Brightness,
        SystemMode,

        CustomSensor
    };

    Q_ENUM(RequestType)

    /* Public Functions
     * ****************************************************************************************/
    //! Create new sensor
    Q_INVOKABLE void createSensor(QString name, QString id);


    //! Send requests
    //! todo: transfer data with UARTConnection instance
    Q_INVOKABLE QVariantMap sendRequest(QString className, QString method, QVariantList data);

    /* Public Functions
     * Read and write data without any UART connection
     * Read and write data directly
     * ****************************************************************************************/

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

Q_SIGNALS:
    /* Public Signals
     * ****************************************************************************************/

    //! Send responce with requestType
    void responseReady(int requestType, QVariant response);

private Q_SLOTS:
    /* Private Slots
     * ****************************************************************************************/


private:
    /* Private Functions
     * ****************************************************************************************/
    QVariantMap getMainData();

private:
    /* Attributes
     * ****************************************************************************************/
    QVariantMap _mainData;

    //! Worker thread
    QThread mThread;

    UARTConnection * uartConnection;
};

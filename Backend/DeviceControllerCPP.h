#pragma once

#include <QQmlEngine>
#include <QString>
#include <QtNetwork>

#include "DeviceAPI.h"
#include "deviceIOController.h"

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
    //! Send requests
    //! todo: transfer data with UARTConnection instance
    Q_INVOKABLE QVariantMap sendRequest(QString className, QString method, QVariantList data);

    //! set backlight using uart and respond the success, data should have 5 items
    //! including r, g, b, mode (0 for ui, 1 will be send internally), on/off
    Q_INVOKABLE bool setBacklight(QVariantList data);

    //! set setttings using uart and file and respond the success
    Q_INVOKABLE bool setSettings(QVariantList data);

    /* Public Functions
     * Read and write data without any UART connection
     * Read and write data directly
     * ****************************************************************************************/

    //! starts device
    //! todo: move to constructor later
    Q_INVOKABLE void startDevice();

    //! Stop device
    Q_INVOKABLE void stopDevice();

Q_SIGNALS:
    /* Public Signals
     * ****************************************************************************************/

    //! Send alert to ui
    void alert(STHERM::AlertLevel alertLevel,
               STHERM::AlertTypes alertType,
               QString alertMessage = QString());

private Q_SLOTS:
    /* Private Slots
     * ****************************************************************************************/

private:
    /* Private Functions
     * ****************************************************************************************/
    QVariantMap getMainData();

    /* Attributes
     * ****************************************************************************************/
    QVariantMap _mainData;

    DeviceIOController *_deviceIO;
    DeviceAPI *_deviceAPI;
};

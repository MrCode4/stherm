#pragma once

#include <QQmlEngine>
#include <QString>
#include <QtNetwork>

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

    DeviceIOController *_deviceController;
};

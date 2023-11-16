#pragma once

#include <QThread>
#include <QTimer>

#include "DataParser.h"
#include "UARTConnection.h"
#include "UtilityHelper.h"

/*! ***********************************************************************************************
 * This class manages read and write from device using UART
 * todo: Add a request manager (queue)
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
    void mainDataReady(QVariantMap data);

    //! Send response with requestType
    void responseReady(int requestType, QVariant response);

    //! Send alert to ui
    void alert(STHERM::AlertLevel alertLevel,
               STHERM::AlertTypes alertType,
               QString alertMessage = QString());

private:
    void run() override;

    //! Create TI connection, called each 10 seconds, getInfo (mainData,temp, hum, aq, pressure), manage requests, wiring check
    void createTIConnection();

    //! Create NRF connection, called each 30 seconds, tof, sensors, getInfo (?)
    void createNRF();

    //! Configure NRF
    void nrfConfiguration();

    //! Check alerts with AQ_TH_PR_vals
    void checkMainDataAlert(const STHERM::AQ_TH_PR_vals &values);

    //! Process NRF response
    void processNRFResponse(STHERM::SIOPacket rxPacket);

    //! Process TI response
    void processTIResponse(STHERM::SIOPacket rxPacket);

    //! Get time configs from saved configs file.
    STHERM::ResponseTime getTimeConfig();

    //! Get sensors time configs from saved configs file.
    QList<STHERM::SensorTimeConfig> getSensorTimeConfig();

    //! Get sensors from sensor_config file
    QList<STHERM::SensorConfigThresholds> getSensorThresholds();

    //! Check sensor thresholds
    void checkSensorThreshold(STHERM::SensorConfigThresholds &threshold);

    //! Get paired sensors from sensor file.
    QList<STHERM::DeviceType> getPairedSensors();

    void tiConfiguration();
    bool addPendingSensor(STHERM::DeviceType inc);

    //! Get device id
    void getDeviceID();

private:
    //! Device id
    QString mDeviceID;

    DataParser mDataParser;

    UARTConnection *nRfConnection;
    UARTConnection *tiConnection;

    UARTConnection *gpio4Connection;
    UARTConnection *gpio5Connection;

    bool mStopReading;

    //! Paired devices
    QList<STHERM::DeviceType> mDevices;
    STHERM::DeviceType mMainDevice;

    bool nrfWaitForResponse;

    QByteArray mSensorPacketBA;
    QByteArray mTOFPacketBA;

    int brighness_mode;

    STHERM::AQ_TH_PR_thld AQ_TH_PR_thld;

    QList<STHERM::DeviceType> pairedSensors;

    QList<uint8_t> mWiringState;

    //! Fill from get_dynamic1
    QList<uint8_t> relays_in;

    //! Fill in set relay command
    QList<uint8_t> relays_in_l;

    QTimer wtd_timer;
};

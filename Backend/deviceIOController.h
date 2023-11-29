#pragma once

#include <QThread>
#include <QTimer>
#include <queue>

#include "DataParser.h"
#include "UARTConnection.h"
#include "UtilityHelper.h"
#include "GPIO/GpioHandler.h"

/*! ***********************************************************************************************
 * This class manages read and write from device using UART
 * todo: Add a request manager (queue)
 * ************************************************************************************************/
class DeviceIOPrivate;
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

    void updateRelays(STHERM::RelayConfigs relays);

    /* Public Methods
     * ****************************************************************************************/
public:
    bool setBacklight(QVariantList data);
    //! TODO handles only the brighness for now
    bool setSettings(QVariantList data);

signals:
    void mainDataReady(QVariantMap data);

    //! Send response with requestType
    void responseReady(int requestType, QVariant response);

    //! Send alert to controller
    void alert(STHERM::AlertLevel alertLevel,
               STHERM::AlertTypes alertType,
               QString alertMessage = QString());

private slots:
    void wtdExec();
    void wiringExec();
    void nRFExec();

private:
    void initialize();

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
    bool processNRFQueue();

    //! Process TI response
    void processTIResponse(STHERM::SIOPacket rxPacket);
    bool processTIQueue();
    bool sendTIRequest(STHERM::SIOPacket txPacket);

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

    //! Check relay with wiring states
    bool checkRelayVaidation();

private:
    DeviceIOPrivate *m_p;

    UARTConnection *m_nRfConnection;
    UARTConnection *m_tiConnection;
    GpioHandler *m_gpioHandler4;
    GpioHandler *m_gpioHandler5;

    DataParser m_dataParser;

    QTimer m_wtd_timer;
    QTimer m_wiring_timer;
    QTimer m_nRF_timer;

    std::queue<STHERM::SIOPacket> m_nRF_queue;

    std::queue<STHERM::SIOPacket> m_TI_queue;
};

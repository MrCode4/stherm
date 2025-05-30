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
class DeviceIOController : public QObject
{
    Q_OBJECT
public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    explicit DeviceIOController(QObject *parent = nullptr);
    ~DeviceIOController();

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

    //! Stop reading data from device
    void stopReading();

    //! Update paired sensors in TI
    void updateTiDevices();

    bool update_nRF_Firmware();

    void updateRelays(STHERM::RelayConfigs relays, bool force = false);
    bool testRelays(QVariantList relaysData);

    double backlightFactor();

    /* Public Methods
     * ****************************************************************************************/
public:
    bool setBacklight(QVariantList data);

    //! 0 for off, percentage
    bool setFanSpeed(int speed);
    //! TODO handles only the brightness for now
    bool setSettings(QVariantList data);

    void setBrightnessTest(int brightness, bool test = true);

    void sendRelays();

    QString getNRF_HW() const;

    QString getNRF_SW() const;

    QString getTI_HW() const;

    QString getTI_SW() const;

    //! Start TOF sensor reading
    void startTOFGpioHandler();
    //! TODO: Enhance TOF sensor data processing speed
    //!  and resource utilization by streamlining data
    //!  reading and process and other operations.
    //!  we can implement a power saving measures
    //!  by dynamically enabling/disabling the sensor based
    //!  on device state (e.g., locked/unlocked).
    void stopTOFGpioHandler();

signals:
    void mainDataReady(QVariantMap data);
    void tofDataReady(QVariantMap data);

    //! Send response with requestType
    void responseReady(int requestType, QVariant response);

    //! Send alert to controller
    void alert(STHERM::AlertLevel alertLevel,
               AppSpecCPP::AlertTypes alertType,
               QString alertMessage = QString());

    void tiVersionUpdated();
    void nrfVersionUpdated();

    void adaptiveBrightness(double adaptiveBrightness);

    void fanStatusUpdated(bool off);

    void relaysUpdated(STHERM::RelayConfigs relays);

    //! The system set to off when humidity or temperature sensors malfunction, so send true
    //! Exit from Force off mode when the sensors work properly, so send false
    void forceOffSystem(bool forceOff = false);

    //! co2SensorStatus transmits CO2 sensor health.
    //! True indicates proper operation,
    //!  False indicates malfunction.
    void co2SensorStatus (bool status = true);

    void temperatureSensorStatus(bool status = true);
    void humiditySensorStatus(bool status = true);

private slots:
    void wtdExec();
    void wiringExec();
    void nRFExec();

private:
    void initialize();

    //! Create TI connection, called each 10 seconds, getInfo (mainData,temp, hum, aq, pressure), manage requests, wiring check
    void createTIConnection();

    //! Create NRF connection, called each 30 seconds, tof, sensors, getInfo (?)
    void createNRF();

    //! Configure NRF
    void nrfConfiguration();

    //! Check alerts with AQ_TH_PR_vals
    //! fanSpeed default is 4000 to avoid any related alerts.
    void checkMainDataAlert(const STHERM::AQ_TH_PR_vals &values, const uint16_t &fanSpeed = 4000, const uint32_t luminosity = 100);

    //! Process NRF response
    void processNRFResponse(STHERM::SIOPacket rxPacket, const STHERM::SIOPacket &txPacket);
    bool processNRFQueue(STHERM::SIOCommand cause = STHERM::NoCommand);

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

    void checkTOFRangeValue(uint16_t range_mm);
    void checkTOFLuminosity(uint32_t luminosity);

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
    QTimer m_adaptiveBrightness_timer;

    QString NRF_HW;
    QString NRF_SW;
    QString TI_HW;
    QString TI_SW;

    std::queue<STHERM::SIOPacket> m_nRF_queue;

    std::queue<STHERM::SIOPacket> m_TI_queue;


    double m_backlightFactor = 0.0;
    QTimer m_backlightFactorUpdater;

    QElapsedTimer m_TemperatureAlertET;
    QElapsedTimer m_HumidityAlertET;
    QElapsedTimer m_FanAlertET;
    QElapsedTimer m_LightAlertET;

    //! Verifies if data from sensors like humidity, temperature, and others is received.
    QTimer mSensorDataRecievedTimer;

    //! Has correct sensor data been received from the humidity and temperature sensor
    bool mIsHumTempSensorValid;

    //! Is there recent data available from the sensors
    bool mIsDataReceived;

    QTimer mGetInfoNRFStarter;
    QTimer mGetInfoTIStarter;
};

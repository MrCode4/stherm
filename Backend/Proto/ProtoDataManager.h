#pragma once

#include <QObject>
#include <QQmlEngine>

#include "DevApiExecutor.h"

#include "streamdata.pb.h"

class ProtoDataManager : public DevApiExecutor
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit ProtoDataManager(QObject *parent = nullptr);
    ~ProtoDataManager();

private:
    //! Change mode
    enum ChangeMode {
        CMNone                = 0,
        CMSetTemperature      = 1 << 0,
        CMSetHumidity         = 1 << 1,
        CMCurrentTemperature  = 1 << 2,
        CMCurrentHumidity     = 1 << 3,
        CMMCUTemperature      = 1 << 4,
        CMAirPressure         = 1 << 5,
        CMCurrentAirQuality   = 1 << 6,
        CMCurrentCoolingStage = 1 << 7,
        CMCurrentHeatingStage = 1 << 8,
        CMCurrentFanStatus    = 1 << 9,
        CMLedStatus           = 1 << 10,
        CMAll                 = CMSetTemperature | CMSetHumidity | CMCurrentTemperature | CMCurrentHumidity |
                                CMMCUTemperature | CMAirPressure | CMCurrentAirQuality | CMCurrentCoolingStage |
                                CMCurrentHeatingStage | CMCurrentFanStatus | CMLedStatus
    };

public:


    Q_INVOKABLE void setSetTemperature(const double &tempratureC);
    Q_INVOKABLE void setSetHumidity(const double &humidity);
    Q_INVOKABLE void setCurrentTemperature(const double &tempratureC);
    Q_INVOKABLE void setCurrentHumidity(const double &humidity);
    Q_INVOKABLE void setMCUTemperature(const double &mcuTempratureC);
    Q_INVOKABLE void setAirPressure(const int &airPressureHPa); //TODO
    Q_INVOKABLE void setCurrentAirQuality(const int &airQuality);
    Q_INVOKABLE void setCurrentCoolingStage(const int &coolingStage);
    Q_INVOKABLE void setCurrentHeatingStage(const int &heatingStage);
    Q_INVOKABLE void setCurrentFanStatus(const bool &fanStatus);
    Q_INVOKABLE void setLedStatus(const bool &ledStatus);

private:
    LiveDataPoint *addNewPoint();

    void logStashData();

    void updateChangeMode(ChangeMode cm);

    //! Create the binary file
    void generateBinaryFile();

    //! Send the binary data to server
    void sendDataToServer();

private:
        /*
         * Array of points:
         *      time                          - timestamp in seconds in UTC
         *      set_temperature               - target set temperature (set manually or by scheduler)
         *      set_humidity                  - target humidity (set manually or by scheduler)
         *      current_temperature_embedded  - embedded temperature sensor value in Celsius
         *      current_humidity_embedded     - embedded humidity sensor value in %
         *      current_temperature_MCU       - CPU temperature in Celsius
         *      air_pressure_embedded         - embedded pressure sensor value in hPa
         *      current_air_quality           - air quality indicator (1,2 or 3)
         *      current_cooling_stage         - currently running cooling stage (0 means cooling is off)
         *      current_heating_stage         - currently running heating stage (0 means heating is off)
         *      current_fan_status            - fan state (0 means is off)
         *      led_status                    - led lights state (0 means is off)
         *      is_sync                       - indication of synchronization packet (package should contain all 11 values). Should be at least every hour.
        */
    LiveDataPointList mLiveDataPointList;

    //! Keep the latest data values.
    LiveDataPoint *mLateastDataPoint;

    QTimer mSenderTimer;
    QTimer mDataPointLogger;
    QTimer mCreatGeneralBufferTimer;

    //! Flag to stash data
    int mChangeMode;
};


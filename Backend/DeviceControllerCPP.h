#pragma once

#include <QQmlEngine>
#include <QString>
#include <QtNetwork>

#include "Core/System.h"
#include "DeviceAPI.h"
#include "deviceIOController.h"
#include "Core/Scheme.h"
#include "Device/SystemSetup.h"

class ScheduleCPP;

/*! ***********************************************************************************************
 * This class manages send requests from app to device and and process the received response.
 * ************************************************************************************************/

class DeviceControllerCPP  : public QObject
{
    Q_OBJECT

    Q_PROPERTY(SystemSetup *systemSetup READ systemSetup WRITE setSystemSetup NOTIFY systemSetupChanged FINAL)
    Q_PROPERTY(NUVE::System *system MEMBER  m_system NOTIFY systemChanged)
    Q_PROPERTY(DeviceAPI    *deviceAPI MEMBER  _deviceAPI NOTIFY deviceAPIChanged)
    //Q_PROPERTY(SystemSetup *systemSetup READ systemSetup WRITE setSystemSetup NOTIFY systemSetupChanged FINAL)

    QML_ELEMENT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/
    DeviceControllerCPP(QObject *parent = nullptr);
    ~DeviceControllerCPP();


    static DeviceControllerCPP* instance();

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

    //!
    Q_INVOKABLE QVariantMap getMainData();

    //! set backlight using uart and respond the success, data should have 5 items
    //! including r, g, b, mode (0 for ui, 1 will be send internally), on/off
    //! isScheme: is true when the backlight set from scheme and false for model
    Q_INVOKABLE bool setBacklight(QVariantList data, bool isScheme = false);

    //! set settings using uart and file and respond the success
    Q_INVOKABLE bool setSettings(QVariantList data);

    //! update vacation
    Q_INVOKABLE void setVacation(const double min_Temperature, const double max_Temperature,
                                 const double min_Humidity,    const double max_Humidity);

    Q_INVOKABLE void setRequestedTemperature(const double temperature);
    Q_INVOKABLE void setRequestedHumidity(const double humidity);

    //! set relays using uart and file and respond the success
    Q_INVOKABLE bool setTestRelays(QVariantList data);

    Q_INVOKABLE void setOverrideMainData(QVariantMap mainDataOverride);

    //! Set fan to scheme
    Q_INVOKABLE bool setFan(AppSpecCPP::FanMode fanMode, int newFanWPH);

    /* Public Functions
     * Read and write data without any UART connection
     * Read and write data directly
     * ****************************************************************************************/

    //! starts device
    //! todo: move to constructor later
    Q_INVOKABLE void startDevice();

    //! Stop device
    Q_INVOKABLE void stopDevice();

    Q_INVOKABLE void setActivatedSchedule(ScheduleCPP* schedule);

    Q_INVOKABLE int getStartMode();

    SystemSetup* systemSetup() const;
    void setSystemSetup (SystemSetup* systemSetup);

Q_SIGNALS:
    /* Public Signals
     * ****************************************************************************************/

    //! Send alert to ui
    void alert(STHERM::AlertLevel alertLevel,
               STHERM::AlertTypes alertType,
               QString alertMessage = QString());

    void systemSetupChanged();

    void systemChanged();

    void deviceAPIChanged();

private:
    // update main data and send data to scheme.
    void setMainData(QVariantMap mainData);
    static DeviceControllerCPP* sInstance;

private Q_SLOTS:
    /* Private Slots
     * ****************************************************************************************/

private:
    /* Private Functions
     * ****************************************************************************************/

private:
    /* Attributes
     * ****************************************************************************************/
    QVariantMap _mainData;
    QVariantMap _mainData_override;
    bool _override_by_file = false;
    double _temperatureLast = 0.0;

    DeviceIOController *_deviceIO;
    DeviceAPI *_deviceAPI;

    SystemSetup *mSystemSetup;
    Scheme      *m_scheme;

    NUVE::System *m_system;

    QTimer mBacklightTimer;
    QTimer mBacklightPowerTimer;

    QVariantList mBacklightModelData;

    //! Temperature correction parameters
    double mDeltaTemperatureIntegrator;
    const double TEMPERATURE_INTEGRATOR_DECAY_CONSTANT = 0.99721916;
    const double TEMPERATURE_COMPENSATION_OFFSET = 0.5;
    const double TEMPERATURE_COMPENSATION_SCALER = 0.8*3.1/360;
    double deltaCorrection()
    {
        return  TEMPERATURE_COMPENSATION_OFFSET + mDeltaTemperatureIntegrator * TEMPERATURE_COMPENSATION_SCALER;
    }
};

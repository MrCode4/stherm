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

    Q_PROPERTY(QString    swTI  READ  getTI_SW  NOTIFY tiVersionChanged)
    Q_PROPERTY(QString    hwTI  READ  getTI_HW  NOTIFY tiVersionChanged)
    Q_PROPERTY(QString    swNRF READ  getNRF_SW NOTIFY nrfVersionChanged)
    Q_PROPERTY(QString    hwNRF READ  getNRF_HW NOTIFY nrfVersionChanged)

    Q_PROPERTY(double  adaptiveBrightness READ  adaptiveBrightness NOTIFY adaptiveBrightnessChanged)

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

    //!
    Q_INVOKABLE void writeTestResult(const QString& testName, const QString& testResult, const QString& description="");
    Q_INVOKABLE void writeTestResult(const QString& testName, bool testResult, const QString& description="");
    Q_INVOKABLE void beginTesting();

    Q_INVOKABLE void testFinished();
    Q_INVOKABLE bool getSNTestMode();

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

    Q_INVOKABLE bool updateNRFFirmware();

    Q_INVOKABLE bool checkNRFFirmwareVersion();

    Q_INVOKABLE void setAutoMinReqTemp(const double min);
    Q_INVOKABLE void setAutoMaxReqTemp(const double max);

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

    Q_INVOKABLE bool getUpdateMode();

    Q_INVOKABLE bool checkSN();

    Q_INVOKABLE void checkContractorInfo();

    Q_INVOKABLE void pushSettingsToServer(const QVariantMap &settings, bool hasSettingsChanged);


    SystemSetup* systemSetup() const;
    void setSystemSetup (SystemSetup* systemSetup);

    //! Reset relays based on model
    Q_INVOKABLE void sendRelaysBasedOnModel();

    QString getNRF_HW() const;

    QString getNRF_SW() const;

    QString getTI_HW() const;

    QString getTI_SW() const;

    double adaptiveBrightness();

    Q_INVOKABLE void nightModeControl(bool start);
    Q_INVOKABLE void setCPUGovernor(AppSpecCPP::CPUGovernerOption CPUGovernerOption);

    //! Forget device and system settings
    Q_INVOKABLE void forgetDevice();

    Q_INVOKABLE bool checkUpdateMode();

Q_SIGNALS:
    /* Public Signals
     * ****************************************************************************************/

    //! Send alert to ui
    void alert(STHERM::AlertLevel alertLevel,
               AppSpecCPP::AlertTypes alertType,
               QString alertMessage = QString());

    void systemSetupChanged();

    void systemChanged();

    void deviceAPIChanged();

    void contractorInfoUpdated(QString brandName, QString phoneNumber, QString iconUrl, QString url, QString techUrl);

    void snModeChanged(int snMode);

    void startModeChanged(int startMode);

    void tiVersionChanged();
    void nrfVersionChanged();

    void nrfUpdateStarted();

    void fanWorkChanged(bool fanState);
    void currentSystemModeChanged(AppSpecCPP::SystemMode fanState);

    void adaptiveBrightnessChanged();

    //! Forward signals from Scheme and send to UI
    void startSystemDelayCountdown(AppSpecCPP::SystemMode mode, int delay);
    void stopSystemDelayCountdown();

private:
    // update main data and send data to scheme.
    void setMainData(QVariantMap mainData);
    static DeviceControllerCPP* sInstance;

    void startTestMode();

    void setAdaptiveBrightness(const double adaptiveBrightness);

    //! return true: fan is ON
    //! return false: fan is OFF
    bool isFanON();

private Q_SLOTS:
    /* Private Slots
     * ****************************************************************************************/
    void processBackdoorSettingFile(const QString &path);

private:
    /* Private Functions
     * ****************************************************************************************/
    void writeGeneralSysData(const QStringList &cpuData, const int &brightness);

    void setFanSpeed(int speed);

    QJsonObject processJsonFile(const QString &path, const QStringList &requiredKeys);
    void processBackLightSettings(const QString &path);
    void processFanSettings(const QString &path);
    void processBrightnessSettings(const QString &path);
    QByteArray defaultSettings(const QString &path);

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

    QString m_backdoorPath = "/usr/local/bin/backdoor/";
    QStringList m_watchFiles = { "backlight.json", "brightness.json", "fan.json" };
    QFileSystemWatcher m_fileSystemWatcher;

    QTimer mBacklightTimer;
    QTimer mBacklightPowerTimer;

    // initialized in startup onStartDeviceRequested in qml
    QVariantList mBacklightModelData;
    QVariantList mBacklightActualData;  // for logging purpose
    QVariantList mSettingsModelData;

    QTimer mNightModeTimer;

    //! TODO: Delete when logging is not required
    QTimer mLogTimer;

    QString mGeneralSystemDatafilePath;

    bool mIsNightModeRunning;

    int mFanSpeed;
    bool mFanOff;

    //! TEMP, To keep raw temperature.
    double mRawTemperature;

    //! percent value
    double mAdaptiveBrightness;

    QTimer mTEMPERATURE_COMPENSATION_Timer;

    // Unit: Celsius
    double mTEMPERATURE_COMPENSATION_T1 = 0.2;

    //! Temperature correction parameters
    double mDeltaTemperatureIntegrator;
    const double TEMPERATURE_INTEGRATOR_DECAY_CONSTANT = 0.99721916;
    // TW - remove the doubled up offset and update the 1F
    // const double TEMPERATURE_COMPENSATION_OFFSET = 0.25 + 2.0 / 1.8;
    const double TEMPERATURE_COMPENSATION_OFFSET = 1.0 / 1.8;
    const double TEMPERATURE_COMPENSATION_SCALER = 0.8 * 3.1 / 360;
    double deltaCorrection()
    {
        return  TEMPERATURE_COMPENSATION_OFFSET + mDeltaTemperatureIntegrator * TEMPERATURE_COMPENSATION_SCALER;
    }

    // Testing
    bool mAllTestsPassed = true;

    AppSpecCPP::CPUGovernerOption mCPUGoverner = AppSpecCPP::CPUGUnknown;
};

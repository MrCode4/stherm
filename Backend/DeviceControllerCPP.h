#pragma once

#include <QQmlEngine>
#include <QString>
#include <QtNetwork>

#include "Core/System.h"
#include "DeviceAPI.h"
#include "deviceIOController.h"
#include "Core/Scheme.h"
#include "HumidityScheme.h"
#include "Device/SystemSetup.h"
#include "Property.h"

class ScheduleCPP;

/*! ***********************************************************************************************
 * This class manages send requests from app to device and and process the received response.
 * ************************************************************************************************/

class DeviceControllerCPP  : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    PROPERTY_PRI(NUVE::Sync*, sync)

    Q_PROPERTY(SystemSetup *systemSetup READ systemSetup WRITE setSystemSetup NOTIFY systemSetupChanged FINAL)
    Q_PROPERTY(NUVE::System *system MEMBER  m_system NOTIFY systemChanged)
    Q_PROPERTY(DeviceAPI    *deviceAPI MEMBER  _deviceAPI NOTIFY deviceAPIChanged)

    Q_PROPERTY(QString    swTI  READ  getTI_SW  NOTIFY tiVersionChanged)
    Q_PROPERTY(QString    hwTI  READ  getTI_HW  NOTIFY tiVersionChanged)
    Q_PROPERTY(QString    swNRF READ  getNRF_SW NOTIFY nrfVersionChanged)
    Q_PROPERTY(QString    hwNRF READ  getNRF_HW NOTIFY nrfVersionChanged)

    // for quiet mode bind
    Q_PROPERTY(double  adaptiveBrightness READ  adaptiveBrightness NOTIFY adaptiveBrightnessChanged)

    Q_PROPERTY(bool  isNeedOutdoorTemperature READ  isNeedOutdoorTemperature NOTIFY isNeedOutdoorTemperatureChanged)
    Q_PROPERTY(bool  isEligibleOutdoorTemperature READ  isEligibleOutdoorTemperature NOTIFY isEligibleOutdoorTemperatureChanged)
    Q_PROPERTY(bool  isZipCodeValid READ  isZipCodeValid NOTIFY isZipCodeValidChanged)

    //Q_PROPERTY(SystemSetup *systemSetup READ systemSetup WRITE setSystemSetup NOTIFY systemSetupChanged FINAL)



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
    Q_INVOKABLE double getTemperature();

    //!
    Q_INVOKABLE void writeTestResult(const QString &fileName, const QString& testName, const QString& testResult, const QString& description="");
    Q_INVOKABLE void saveTestResult(const QString& testName, bool testResult, const QString& description="");
    Q_INVOKABLE QString beginTesting();

    Q_INVOKABLE void testBrightness(int value);
    Q_INVOKABLE void stopTestBrightness();

    Q_INVOKABLE void testFinished();
    Q_INVOKABLE bool getSNTestMode();

    //! set backlight using uart and respond the success, data should have 5 items
    //! including r, g, b, mode (0 for ui, 1 will be send internally), on/off
    //! isScheme: is true when the backlight set from scheme and false for model
    Q_INVOKABLE bool setBacklight(QVariantList data, bool isScheme = false);

    //! set settings using uart and file and respond the success
    Q_INVOKABLE bool setSettings(QVariantList data);
    Q_INVOKABLE void setCelsius(bool isCelsius);

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

    Q_INVOKABLE void setAutoMinReqTemp(const double cel_value);
    Q_INVOKABLE void setAutoMaxReqTemp(const double cel_value);

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

    Q_INVOKABLE void pushSettingsToServer(const QVariantMap &settings);

    Q_INVOKABLE void setEndpoint(const QString &subdomain, const QString &domain);
    Q_INVOKABLE QString getEndpoint();

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

    //! Reset Device to Factory setting
    Q_INVOKABLE void resetToFactorySetting();

    //! TODO
    //! Maybe call from server
    Q_INVOKABLE void setSampleRate(const int sampleRate);

    Q_INVOKABLE bool checkUpdateMode();

    Q_INVOKABLE void pushAutoSettingsToServer(const double& auto_temp_low, const double& auto_temp_high);

    Q_INVOKABLE void wifiConnected(bool hasInternet);

    Q_INVOKABLE void lockDeviceController(bool isLock);

    //! Start/Stop the temperature scheme
    //! start = true, start scheme
    //! start = false, stop scheme
    Q_INVOKABLE void runTemperatureScheme(bool start);

    //! Start/Stop the humidity scheme
    //! start = true, start scheme
    //! start = false, stop scheme
    Q_INVOKABLE void runHumidityScheme(bool start);

    Q_INVOKABLE void switchDFHActiveSysType(AppSpecCPP::SystemType activeSystemType);

    Q_INVOKABLE bool isTestsPassed();
    void doPerfTest(AppSpecCPP::SystemMode mode);
    void revertPerfTest();

    Q_INVOKABLE double effectiveHumidity();

Q_SIGNALS:
    /* Public Signals
     * ****************************************************************************************/

    void tempSchemeStateChanged(bool started);
    void humiditySchemeStateChanged(bool started);

    //! Send alert to ui
    void alert(STHERM::AlertLevel alertLevel,
               AppSpecCPP::AlertTypes alertType,
               QString alertMessage = QString());

    void auxiliaryStatusChanged(bool running);

    void systemSetupChanged();

    void systemChanged();

    void deviceAPIChanged();

    void contractorInfoUpdated(QString brandName, QString phoneNumber, QString iconUrl, QString url, QString techUrl);

    void snModeChanged(int snMode);

    void startModeChanged(int startMode);
    void actualModeStarted(AppSpecCPP::SystemMode mode);

    void tiVersionChanged();
    void nrfVersionChanged();

    void nrfUpdateStarted();

    void fanWorkChanged(bool fanState);
    void currentSystemModeChanged(AppSpecCPP::SystemMode obState, int currentHeatingStage, int currentCoolingStage);

    //! Active system mode changed due to dual fuel heating
    void dfhSystemTypeChanged(AppSpecCPP::SystemType activeSystemType);

    void adaptiveBrightnessChanged();

    //! Forward signals from Scheme and send to UI
    void startSystemDelayCountdown(AppSpecCPP::SystemMode mode, int delay);
    void stopSystemDelayCountdown();

    //! The system set to off when humidity or temperature sensors malfunction
    void forceOffSystem();

    //! Exit from Force off mode when the sensors work properly
    void exitForceOffSystem();

    //! co2SensorStatus transmits CO2 sensor health.
    //! True indicates proper operation,
    //!  False indicates malfunction.
    void co2SensorStatus (bool status = true);

    void temperatureSensorStatus(bool status = true);
    void humiditySensorStatus(bool status = true);

    void monitoringTemperatureUpdated(double monitoringTempratureC);

    //! To block mode change in UI
    void manualEmergencyModeUnblockedAfter(int secs);

    void isNeedOutdoorTemperatureChanged();
    void isEligibleOutdoorTemperatureChanged();
    void isZipCodeValidChanged();

    void emulateWarrantyFlow();

private:
    // update main data and send data to scheme.
    void setMainData(QVariantMap mainData, bool addToData = false);
    static DeviceControllerCPP* sInstance;

    //! Start the factory test mode without the TI board attached.
    void startTestMode();

    void publishTestResults(const QString &resultsPath);

    void setAdaptiveBrightness(const double adaptiveBrightness);

    //! return true: fan is ON
    //! return false: fan is OFF
    bool isFanON();

    void writeSensorData(const QVariantMap &data);

    void updateIsNeedOutdoorTemperature();
    bool isNeedOutdoorTemperature();

    void updateIsEligibleOutdoorTemperature();
    bool isEligibleOutdoorTemperature();

    void updateZipCodeValidation(const bool &isValid);
    bool isZipCodeValid();

private Q_SLOTS:
    /* Private Slots
     * ****************************************************************************************/
    void processBackdoorSettingFile(const QString &path);
    void onCurrentSystemModeChanged(AppSpecCPP::SystemMode obState,
                                    int currentHeatingStage,
                                    int currentCoolingStage);
    void saveTempratures();

private:
    /* Private Functions
     * ****************************************************************************************/
    void writeGeneralSysData(const QStringList &cpuData, const int &brightness);

    void setFanSpeed(int speed);

    QJsonObject processJsonFile(const QString &path, const QStringList &requiredKeys);
    void processBackLightSettings(const QString &path);
    void processFanSettings(const QString &path);
    void processNightModeControlSettings(const QString &path);
    void processBrightnessSettings(const QString &path);
    void processRelaySettings(const QString &path);
    void processEmulateWarrantyFlow(const QString &path);
    QByteArray defaultSettings(const QString &path);
    bool writeDefaultSettings(const QString &path);

    //! Start/Stop the timer for get the outdoor temperature
    void checkForOutdoorTemperature();

    double calculateProcessedTemperature(const double &temperatureC) const;

    void loadTempratures();

private:
    /* Attributes
     * ****************************************************************************************/
    // Store the raw main data
    QVariantMap _lastMainData;
    QVariantMap _mainData;
    QVariantMap _mainData_override;
    bool _override_by_file = false;
    double _temperatureLast = 0.0;
    bool mIsCelsius = false;

    bool mIsDeviceStarted = false;

    DeviceIOController *_deviceIO;
    DeviceAPI *_deviceAPI;

    SystemSetup *mSystemSetup;
    AppSpecCPP::SystemMode mActiveSystemMode;

    //! Create a shared instance of SchemeDataProvider to
    //! provide data for scheme and HumidittScheme
    QSharedPointer<SchemeDataProvider> mSchemeDataProvider;

    //! Object to manage temperature control
    Scheme* mTempScheme;

    //! Object to manage humidity control
    HumidityScheme* mHumidityScheme;

    QTimer mGetOutdoorTemperatureTimer;

    NUVE::System *m_system;

    QString m_backdoorPath = "/usr/local/bin/backdoor/";
    QStringList m_watchFiles = { "backlight.json", "brightness.json", "fan.json" , "relays.json", "emulateWarrantyFlow.json", "nightMode.json" };
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
    QTimer mSaveSensorDataTimer;
    QElapsedTimer mResponsivenessTimer;

    QString mGeneralSystemDatafilePath;

    bool mIsNightModeRunning;

    bool mDeviceHasInternet;

    //! Is the outdoor temperature required?
    bool mIsNeedOutdoorTemperature;

    //! Check the feasibility of sending outdoor temperature request.
    bool mIsEligibleOutdoorTemperature;

    bool mIsZipCodeValid;

    int mFanSpeed;
    bool mFanOff;

    //! percent value
    double mAdaptiveBrightness;

    QTimer mTEMPERATURE_COMPENSATION_Timer;

    // Unit: Celsius
    double mTEMPERATURE_COMPENSATION_T1 = 0.2;

    //! Temperature correction parameters
    double mDeltaTemperatureIntegrator = 0;
    const double TEMPERATURE_INTEGRATOR_DECAY_CONSTANT = 0.99721916;
    // TW - remove the doubled up offset and update the 1F
    // const double TEMPERATURE_COMPENSATION_OFFSET = 0.25 + 2.0 / 1.8;
    const double TEMPERATURE_COMPENSATION_OFFSET = 1.0 / 1.8;
    const double TEMPERATURE_COMPENSATION_SCALER = 0.8 * 3.1 / 360;
    //! in Celcius
    double deltaCorrection()
    {
        double correction = TEMPERATURE_COMPENSATION_OFFSET + mDeltaTemperatureIntegrator * TEMPERATURE_COMPENSATION_SCALER;
        // the hypothesis is correction value is in F and should be converted
        return  correction;
    }

    QTimer mSaveTemperatureTimer;
    QTimer mLoadTemperatureTimer;

    // Testing
    std::map<QString, bool> mAllTestsResults;
    std::map<QString, QString> mAllTestsValues;
    //! TODO: initialize all tests as a test for all tests to be conducted
    QStringList mAllTestNames; // to keep them in order

    AppSpecCPP::CPUGovernerOption mCPUGoverner = AppSpecCPP::CPUGUnknown;

    STHERM::RelayConfigs mRelaysUpdated;

    bool mBackdoorSchemeEnabled = false;
};

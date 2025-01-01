#pragma once

#include <QObject>
#include <QtNetwork>
#include <QQmlEngine>

#include "Backend/Device/nuve_types.h"
#include "RestApiExecutor.h"
#include "Sync.h"

/*! ***********************************************************************************************
 * This class manage system requests.
 * ************************************************************************************************/
namespace NUVE {

using fileSenderCallback = std::function<void(QString error)>;

class senderProcess : public QProcess
{
    Q_OBJECT
public:
    senderProcess() {}
    void initialize(std::function<void(QString)> errorHandler, const QString &subject, const QString &joiner = " <br>");
    virtual ~senderProcess() {}

    bool busy() const {
        return state() != QProcess::NotRunning || !mCallbacks.isEmpty();
    }

    QStringList keys() {
        return mCallbacks.keys();
    }

    void setRole(const QString& role, fileSenderCallback callback = nullptr)
    {
        setProperty("role", role);
        mCallbacks.insert(role, callback);
    }

private:
    QHash<QString, fileSenderCallback> mCallbacks;
    std::function<void(QString)> mErrorHandler = nullptr;
    QString mJoiner;
    QString mSubject;
};

class System : public RestApiExecutor
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QStringList availableVersions READ availableVersions NOTIFY availableVersionsChanged FINAL)
    Q_PROPERTY(QString lastInstalledUpdateDate READ lastInstalledUpdateDate NOTIFY lastInstalledUpdateDateChanged FINAL)
    Q_PROPERTY(QString latestVersion          READ latestVersion           NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionDate      READ latestVersionDate       NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionChangeLog READ latestVersionChangeLog  NOTIFY logVersionChanged FINAL)
    Q_PROPERTY(QString remainingDownloadTime  READ remainingDownloadTime   NOTIFY remainingDownloadTimeChanged FINAL)
    Q_PROPERTY(QString serialNumber           READ serialNumber            NOTIFY serialNumberReady FINAL)
    Q_PROPERTY(QString systemUID              READ systemUID               NOTIFY systemUIDChanged FINAL)
    Q_PROPERTY(QString backdoorLog            READ backdoorLog             NOTIFY backdoorLogChanged FINAL)
    Q_PROPERTY(bool areSettingsFetched  READ areSettingsFetched   NOTIFY areSettingsFetchedChanged FINAL)
    Q_PROPERTY(bool updateAvailable  READ updateAvailable   NOTIFY updateAvailableChanged FINAL)
    Q_PROPERTY(bool testMode         READ testMode WRITE setTestMode   NOTIFY testModeChanged FINAL)
    Q_PROPERTY(bool isManualUpdate   READ isManualMode  NOTIFY isManualModeChanged FINAL)
    //! Enable/disable alert feature
    Q_PROPERTY(bool controlAlertEnabled   READ controlAlertEnabled  NOTIFY controlAlertEnabledChanged FINAL)
    //! Maybe used in future...
    Q_PROPERTY(bool hasForceUpdate    READ hasForceUpdate   NOTIFY forceUpdateChanged FINAL)
    Q_PROPERTY(int partialUpdateProgress      READ partialUpdateProgress    NOTIFY partialUpdateProgressChanged FINAL)

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/

    System(NUVE::Sync *sync, QObject *parent = nullptr);

    ~System();


    /* Public Functions (Setters and getters)
     * ****************************************************************************************/

    //! Reboot device
    Q_INVOKABLE void rebootDevice();

    //! Get log by version
    Q_INVOKABLE QString getLogByVersion(const QString version);

    //! Reset device by exiting app and disabling service
    Q_INVOKABLE void stopDevice();

    Q_INVOKABLE bool fetchSettings();

    Q_INVOKABLE bool hasClient() const;
    bool areSettingsFetched() const;

    //! Get serial number from server
    Q_INVOKABLE void fetchSerialNumber(const QString& uid, bool notifyUser = true);

    //! Send request job to web server
    void requestJob(QString type);

    //! Start the partilally update
    //! if the version is empty, partialUpdate start to install the latest version
    Q_INVOKABLE void partialUpdate(const bool isBackdoor = false);
    Q_INVOKABLE void partialUpdateByVersion(const QString version);

    Q_INVOKABLE void updateAndRestart(const bool isBackdoor, const bool isResetVersion = false,
                                      const bool isFWServerVersion = false);

    //! Get update information from server
    //! notifyUser: Send notification for user when new update is available
    Q_INVOKABLE void fetchUpdateInformation(bool notifyUser = false);

    Q_INVOKABLE QString fetchUpdateInformationSync(bool notifyUser = false);

    Q_INVOKABLE void fetchBackdoorInformation();

    Q_INVOKABLE void pushSettingsToServer(const QVariantMap &settings);

    Q_INVOKABLE void exitManualMode();

    Q_INVOKABLE void ignoreManualUpdateMode(bool checkUpdate = false);

    Q_INVOKABLE bool isFWServerUpdate();

    Q_INVOKABLE void setLimitedModeRemainigTime(const int &limitedModeRemainigTime);
    Q_INVOKABLE int  limitedModeRemainigTime();

    Q_INVOKABLE void setInitialSetupWithNoWIFI(const bool &initialSetupNoWIFI);
    Q_INVOKABLE bool initialSetupWithNoWIFI();

    void wifiConnected(bool hasInternet);

    //! Getters
    QStringList availableVersions();

    QString latestVersion();

    QString latestVersionDate();

    QString latestVersionChangeLog();

    QString remainingDownloadTime();

    QString lastInstalledUpdateDate();

    QString serialNumber();

    QString backdoorLog();

    int partialUpdateProgress();

    bool updateAvailable();

    bool testMode();

    //! checks only existance of the sshpass file in /usr/bin
    bool has_sshPass();

    /*!
     * \brief updateSequenceOnStart gets if the app just updated and set the state false so this happens only once
     * remember to call this only in one place on startup so you can manage better
     * \return if after update use case needs to be handled
     */
    bool updateSequenceOnStart();

    bool hasForceUpdate();

    void setTestMode(bool testMode);

    void setControlAlertEnabled(bool enabled);

    void setPartialUpdateProgress(int progress);

    void setUID(NUVE::cpuid_t uid);
    void setSerialNumber(const QString &sn);

    QString systemUID();

    //! Install update service
    Q_INVOKABLE bool installUpdateService();

    //! Install sshpass if not exits or not working,
    //! tries 3 times recursively if fails
    //! returns false if not success
    //! returns true on windows
    Q_INVOKABLE bool installSSHPass(bool recursiveCall = false);

    //! Install update service
    Q_INVOKABLE bool installUpdate_NRF_FW_Service();

    //! Mount update directory
    Q_INVOKABLE bool mountUpdateDirectory();

    //! Mount Recovery directory
    Q_INVOKABLE bool mountRecoveryDirectory();

    //! Mount nrf fw update directory
    Q_INVOKABLE bool mountNRF_FW_Directory();

    Q_INVOKABLE QString kernelBuildVersion();

    Q_INVOKABLE QString rootfsBuildTimestamp();

    Q_INVOKABLE bool findBackdoorVersion(const QString fileName);

    Q_INVOKABLE bool sendLog(bool showAlert = true);

    Q_INVOKABLE void sendFirstRunLog();

    bool sendResults(const QString &filepath, const QString &remoteIP, const QString &remoteUser, const QString &remotePassword, const QString &destination,
                     bool createDirectory = false);

    Q_INVOKABLE void systemCtlRestartApp();


    QStringList cpuInformation();

    //! Return the first cpu temperature: Device has ONE cpu
    Q_INVOKABLE double cpuTemperature();

    bool mountDirectory(const QString targetDirectory, const QString targetFolder);

    //! Check: the directory is valid and has minimum free space
    bool checkDirectorySpaces(const QString directory, const uint32_t minimumSizeBytes = 400000000);

    bool isManualMode();

    Q_INVOKABLE bool isInitialSetup();
    Q_INVOKABLE void setIsInitialSetup(bool isInitailSetup);

    Q_INVOKABLE bool controlAlertEnabled();

    //! Forget device settings and sync settings
    Q_INVOKABLE void forgetDevice();

    //! Remove folders/files like
    //! - /mnt/log/latestVersion/
    //! - /mnt/log/sensor/
    //! - /mnt/log/log/
    //! - /mnt/log/nrf_fw/
    //! - /mnt/log/recovery/
    Q_INVOKABLE bool removeLogPartition();

    //! Manage quiet/night mode in system
    void setNightModeRunning(const bool running);

    //! Push auto mode settings to server
    void pushAutoSettingsToServer(const double &auto_temp_low, const double &auto_temp_high);

    Q_INVOKABLE QString getCurrentTime();

    Q_INVOKABLE void fetchServiceTitanInformation();

    Q_INVOKABLE bool attemptToRunCommand(const QString& command, const QString& tag);

    Q_INVOKABLE void setAlternativeNoWiFiFlow(const bool &alternativeNoWiFiFlow);
    Q_INVOKABLE bool alternativeNoWiFiFlow();

    Q_INVOKABLE bool isBusylogSender() const;
    Q_INVOKABLE void saveNetworkLogs();
    Q_INVOKABLE bool sendNetworkLogs();

    Q_INVOKABLE bool isValidNetworkRequestRestart();
    Q_INVOKABLE void saveNetworkRequestRestart();

protected slots:
    void onSerialNumberReady();
    void onAppDataReady(QVariantMap data);

signals:
    void serialNumberReady();
    void areSettingsFetchedChanged(bool success);
    void settingsReady(QVariantMap settings);
    void appDataReady(QVariantMap settings);

    void autoModeSettingsReady(const QVariantMap& settings, bool isValid);
    void pushFailed();

    void latestVersionChanged();
    void logVersionChanged();
    void partialUpdateProgressChanged();
    void remainingDownloadTimeChanged();
    void updateAvailableChanged();
    void lastInstalledUpdateDateChanged();

    //! Emit when partially update is ready.
    void partialUpdateReady(bool isBackdoor = false, bool isResetToVersion = false, const bool isFWServerVersion = false);

    //! Start download process.
    void downloadStarted();

    void error(QString err);

    void alert(QString msg);

    void testPublishFinished(QString msg = QString());

    //! Emit when need the system move to updating/restarting mode
    void systemUpdating();

    //! Send when new update os available
    void notifyNewUpdateAvailable();

    void testModeChanged();

    void controlAlertEnabledChanged();

    void availableVersionsChanged();

    void systemUIDChanged();

    void backdoorLogChanged();

    void isManualModeChanged();

    void serialNumberChanged();

    void updateNoChecked();

    void autoModePush(bool isSuccess);

    void pushSuccess();

    void testModeStarted();

    //! Pass errors, used for tests in test mode
    //! Use to retry in initial setup
    void fetchUpdateErrorOccurred(QString err);

    void forceUpdateChanged();

    void serviceTitanInformationReady(bool hasError, bool isActive,
                                      QString email, QString zipCode);

    //! Log
    void logAlert(QString msg);
    void logPrepared(bool isSuccess);
    void logSentSuccessfully();
    void sendLogProgressChanged(quint8 percent);

private:
    //! verify dounloaded files and prepare to set up.
    bool verifyDownloadedFiles(QByteArray downloadedData, bool withWrite = true,
                               bool isBackdoor = false, const bool isResetVersion = false,
                               const bool isFWServerVersion = false);


    //! Check new version from file.
    //! This function call automatically.
    //! notifyUser: Send notification for user when new update is available
    //! if installLatestVersion set to true, the latest version will be install
    void checkPartialUpdate(bool notifyUser = false, bool installLatestVersion = false);

    void setUpdateAvailable(bool updateAvailable);


    //! Check and validate update json file
    bool checkUpdateFile(const QByteArray updateData);

    //! Find Latest version from the update JsonObject
    QString findLatestVersion(QJsonObject updateJson);

    //! Update Logs
    void updateLog(const QJsonObject updateJsonObject);

    //! Check force updates
    //! Return first force update version (consider test mode and stage) that in greater than current version, otherwise returns empty string
    QString findForceUpdate(const QJsonObject updateJsonObject);

    //! Update Available versions
    void updateAvailableVersions(const QJsonObject updateJsonObject);

    //! Check and prepare the system to start download process.
    void checkAndDownloadPartialUpdate(const QString installingVersion, const bool isBackdoor = false,
                                       const bool isResetVersion = false, const bool isFWServerVersion = false);

    bool installSystemCtlRestartService();

    //! Update serviceName.service state:
    //! If run = true, enable the service and start it
    //! else disable service
    bool updateServiceState(const QString &serviceName, const bool &run);

    void prepareResultsDirectory(const QString &remoteIP, const QString &remoteUser, const QString &remotePassword, const QString &destination);
    void prepareLogDirectory(fileSenderCallback callback = nullptr);
    void prepareFirstRunLogDirectory();
    QString generateLog();
    void sendFirstRunLogFile();
    bool sendLogFile(bool showAlert = true);
    void sendResultsFile(const QString &filepath, const QString &remoteIP,  const QString &remoteUser, const QString &remotePassword, const QString &destination);
    bool removeDirectory(const QString &path);

    void sendLogToServer(const QStringList &filenames, const bool &showAlert);
    bool checkSendLog(bool showAlert);

private:
    Sync *mSync;

    bool mAreSettingsFetched = false;
    QJsonObject mUpdateJsonObject;

    QByteArray m_expectedUpdateChecksum;

    QStringList mAvailableVersions;

    QString mUpdateFilePath;

    //! Latest version is installable version
    //! (last force update that is greater than current version or the lastes version if no new force version exists)
    QString mLatestVersionKey;

    QString mLatestVersionDate;
    QString mLatestVersionChangeLog;
    QString mLastInstalledUpdateDate;

    bool mIsManualUpdate;
    bool mStartedWithManualUpdate;
    bool mStartedWithFWServerUpdate;

    int mRequiredMemory;
    int mUpdateFileSize;

    QString mRemainingDownloadTime;

    QString mUpdateDirectory;
    QString mRecoveryDirectory;

    int mPartialUpdateProgress;

    bool mUpdateAvailable;

    bool mHasForceUpdate;

    bool mIsInitialSetup;
    bool mControlAlertEnabled;

    //! System on test mode or not
    bool mTestMode;

    bool mIsNightModeRunning;

    //! Set to true after a device restart triggered by a firmware update or reboot,
    //! to prevent firmware update.
    bool mRestarting;

    int sshpassInstallCounter;

    QTimer mFetchActiveTimer;

    QTimer mUpdateTimer;
    QTimer mRetryUpdateTimer;

    NUVE::cpuid_t mUID;

    //! QElapsedTimer to measure download rate.
    QElapsedTimer mElapsedTimer;

    QTimer downloaderTimer;
    bool mIsBusyDownloader = false;
    qint64 mDownloadBytesReceived;
    double mDownloadRateEMA;

    // Backdoor attributes
    QJsonObject mBackdoorJsonObject;
    QString     mBackdoorFileName;
    QString     mBackdoorLog;
    QByteArray  mExpectedBackdoorChecksum;

    int mBackdoorRequiredMemory;
    int mBackdoorUpdateFileSize;

    senderProcess mLogSender;
    senderProcess mFileSender;
    QString mLogRemoteFolder;
    QString mLogRemoteFolderUID;
    QMap<QString, QString> mLastReceivedCommands;
};

} // namespace NUVE

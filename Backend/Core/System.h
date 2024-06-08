#pragma once

#include <QObject>
#include <QtNetwork>
#include <QQmlEngine>

#include "Backend/Device/nuve_types.h"
#include "NetworkWorker.h"
#include "Sync.h"

/*! ***********************************************************************************************
 * This class manage system requests.
 * ************************************************************************************************/
namespace NUVE {
class System : public NetworkWorker
{
    Q_OBJECT

    Q_PROPERTY(QStringList availableVersions READ availableVersions NOTIFY availableVersionsChanged FINAL)

    Q_PROPERTY(QString lastInstalledUpdateDate READ lastInstalledUpdateDate NOTIFY lastInstalledUpdateDateChanged FINAL)
    Q_PROPERTY(QString latestVersion          READ latestVersion           NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionDate      READ latestVersionDate       NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionChangeLog READ latestVersionChangeLog  NOTIFY logVersionChanged FINAL)
    Q_PROPERTY(QString remainingDownloadTime  READ remainingDownloadTime   NOTIFY remainingDownloadTimeChanged FINAL)
    Q_PROPERTY(QString serialNumber           READ serialNumber            NOTIFY snReady FINAL)
    Q_PROPERTY(QString systemUID              READ systemUID               NOTIFY systemUIDChanged FINAL)
    Q_PROPERTY(QString backdoorLog            READ backdoorLog             NOTIFY backdoorLogChanged FINAL)

    Q_PROPERTY(bool updateAvailable  READ updateAvailable   NOTIFY updateAvailableChanged FINAL)
    Q_PROPERTY(bool testMode         READ testMode WRITE setTestMode   NOTIFY testModeChanged FINAL)
    Q_PROPERTY(bool isManualUpdate   READ isManualMode  NOTIFY isManualModeChanged FINAL)
    Q_PROPERTY(bool fetchSuccessOnce   READ hasFetchSuccessOnce  FINAL)


    //! Maybe used in future...
    Q_PROPERTY(bool hasForceUpdate    READ hasForceUpdate   NOTIFY latestVersionChanged FINAL)

    Q_PROPERTY(bool canFetchServer READ canFetchServer WRITE setCanFetchServer NOTIFY canFetchServerChanged FINAL)

    Q_PROPERTY(int partialUpdateProgress      READ partialUpdateProgress    NOTIFY partialUpdateProgressChanged FINAL)

    QML_ELEMENT

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

    //! Get serial number from server
    std::pair<std::string, bool> getSN(cpuid_t accessUid);

    //! Get serial number from server, call from QML and return serial number
    //! Some signals are block in this function.
    Q_INVOKABLE QString getSN(QString accessUid);

    //! Get update
    //! todo: process response packet
    //! TEMP: "022"
    bool getUpdate(QString softwareVersion = "022");

    //! Send request job to web server
    void requestJob(QString type);


    //! Start the partilally update
    //! if the version is empty, partialUpdate start to install the latest version
    Q_INVOKABLE void partialUpdate(const bool isBackdoor = false);
    Q_INVOKABLE void partialUpdateByVersion(const QString version);

    Q_INVOKABLE void updateAndRestart(const bool isBackdoor, const bool isResetVersion = false);

    //! Get update information from server
    //! notifyUser: Send notification for user when new update is available
    Q_INVOKABLE void getUpdateInformation(bool notifyUser = false);

    Q_INVOKABLE void getBackdoorInformation();

    Q_INVOKABLE void pushSettingsToServer(const QVariantMap &settings, bool hasSettingsChanged);

    Q_INVOKABLE void exitManualMode();

    void wifiConnected(bool hasInternet);

    void setCanFetchServer(bool canFetch);

    bool canFetchServer();

    //! Get Contractor Information
    QVariantMap getContractorInfo();

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

    bool has_sshPass();

    /*!
     * \brief updateSequenceOnStart gets if the app just updated and set the state false so this happens only once
     * remember to call this only in one place on startup so you can manage better
     * \return if after update use case needs to be handled
     */
    bool updateSequenceOnStart();

    bool hasForceUpdate();

    void setTestMode(bool testMode);

    void setPartialUpdateProgress(int progress);

    void setUID(NUVE::cpuid_t uid);

    QString systemUID();

    //! Install update service
    Q_INVOKABLE bool installUpdateService();

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

    Q_INVOKABLE void sendLog();

    QStringList cpuInformation();

    bool mountDirectory(const QString targetDirectory, const QString targetFolder);

    bool isManualMode();

    Q_INVOKABLE bool isInitialSetup();
    Q_INVOKABLE void setIsInitialSetup(bool isInitailSetup);

    //! Forget device settings and sync settings
    Q_INVOKABLE void ForgetDevice();

    bool hasFetchSuccessOnce() const;

    //! Manage quiet/night mode in system
    void setNightModeRunning(const bool running);


protected slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

    void onSnReady();

    void createLogDirectoryOnServer();

signals:
    void snReady();
    void settingsReady(QVariantMap settings);
    void pushFailed();

    void latestVersionChanged();
    void logVersionChanged();
    void partialUpdateProgressChanged();
    void remainingDownloadTimeChanged();
    void updateAvailableChanged();
    void lastInstalledUpdateDateChanged();

    //! Emit when partially update is ready.
    void partialUpdateReady(bool isBackdoor = false, bool isResetToVersion = false);

    //! Start download process.
    void downloadStarted();

    void error(QString err);

    void alert(QString msg);

    //! Emit when need the system move to updating/restarting mode
    void systemUpdating();

    //! Send when new update os available
    void notifyNewUpdateAvailable();

    void testModeChanged();

    void availableVersionsChanged();

    void systemUIDChanged();

    void canFetchServerChanged();

    void backdoorLogChanged();

    void isManualModeChanged();

    void serialNumberChanged();

    void updateNoChecked();

    void testModeStarted();

private:

    //! verify dounloaded files and prepare to set up.
    bool verifyDownloadedFiles(QByteArray downloadedData, bool withWrite = true,
                               bool isBackdoor = false, const bool isResetVersion = false);


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
    void checkAndDownloadPartialUpdate(const QString installingVersion, const bool isBackdoor = false, const bool isResetVersion = false);


private:
    Sync *mSync;

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

    int mRequiredMemory;
    int mUpdateFileSize;

    QString mRemainingDownloadTime;

    QString mUpdateDirectory;
    QString mRecoveryDirectory;

    int mPartialUpdateProgress;

    bool mUpdateAvailable;

    bool mHasForceUpdate;

    bool mIsInitialSetup;

    
    //! System on test mode or not
    bool mTestMode;

    bool mIsNightModeRunning;

    QTimer mFetchActiveTimer;

    QTimer mUpdateTimer;

    NUVE::cpuid_t mUID;

    //! QElapsedTimer to measure download rate.
    QElapsedTimer mElapsedTimer;

    QTimer downloaderTimer;

    qint64 mDownloadBytesReceived;
    double mDownloadRateEMA;

    bool mCanFetchServer = true;

    // Backdoor attributes
    QJsonObject mBackdoorJsonObject;
    QString     mBackdoorFileName;
    QString     mBackdoorLog;
    QByteArray  mExpectedBackdoorChecksum;

    int mBackdoorRequiredMemory;
    int mBackdoorUpdateFileSize;

    QProcess mLogSender;
    QString mLogRemoteFolder;
};

} // namespace NUVE

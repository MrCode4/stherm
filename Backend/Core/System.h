#pragma once

#include <QObject>
#include <QtNetwork>
#include <QQmlEngine>

#include "Backend/Device/nuve_types.h"
#include "NetworkWorker.h"

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

    Q_PROPERTY(bool updateAvailable  READ updateAvailable   NOTIFY updateAvailableChanged FINAL)
    Q_PROPERTY(bool testMode         READ testMode WRITE setTestMode   NOTIFY testModeChanged FINAL)

    //! Maybe used in future...
    Q_PROPERTY(bool hasForceUpdate    READ hasForceUpdate   NOTIFY latestVersionChanged FINAL)

    Q_PROPERTY(int partialUpdateProgress      READ partialUpdateProgress    NOTIFY partialUpdateProgressChanged FINAL)

    QML_ELEMENT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/

    System(QObject *parent = nullptr);

    ~System();


    /* Public Functions (Setters and getters)
     * ****************************************************************************************/

    //! Reboot device
    Q_INVOKABLE void rebootDevice();

    //! Get log by version
    Q_INVOKABLE QString getLogByVersion(const QString version);

    //! Reset device by exiting app and disabling service
    Q_INVOKABLE void stopDevice();

    //! Get serial number from server
    std::string getSN(cpuid_t accessUid);

    //! Get update
    //! todo: process response packet
    //! TEMP: "022"
    void getUpdate(QString softwareVersion = "022");

    //! Send request job to web server
    void requestJob(QString type);


    //! Start the partilally update
    //! if the version is empty, partialUpdate start to install the latest version
    Q_INVOKABLE void partialUpdate();
    Q_INVOKABLE void partialUpdateByVersion(const QString version);


    Q_INVOKABLE void updateAndRestart();

    //! Get update information from server
    //! notifyUser: Send notification for user when new update is available
    Q_INVOKABLE void getUpdateInformation(bool notifyUser = false);

    Q_INVOKABLE void wifiConnected(bool hasInternet);

    //! Get Contractor Information
    void getContractorInfo();

    //! Getters
    QStringList availableVersions();

    QString latestVersion();

    QString latestVersionDate();

    QString latestVersionChangeLog();

    QString remainingDownloadTime();

    QString lastInstalledUpdateDate();

    QString serialNumber();

    int partialUpdateProgress();

    bool updateAvailable();

    bool testMode();

    bool hasForceUpdate();

    void setTestMode(bool testMode);

    void setPartialUpdateProgress(int progress);

    void setUID(NUVE::cpuid_t uid);

protected slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

signals:
    void snReady();

    void latestVersionChanged();
    void logVersionChanged();
    void partialUpdateProgressChanged();
    void remainingDownloadTimeChanged();
    void updateAvailableChanged();
    void lastInstalledUpdateDateChanged();

    //! Emit when partially update is ready.
    void partialUpdateReady();

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

private:

    //! verify dounloaded files and prepare to set up.
    bool verifyDownloadedFiles(QByteArray downloadedData, bool withWrite = true);


    //! Check new version from file.
    //! This function call automatically.
    //! notifyUser: Send notification for user when new update is available
    void checkPartialUpdate(bool notifyUser = false);

    //! Mount update directory
    void mountUpdateDirectory();

    void setUpdateAvailable(bool updateAvailable);

    //! Install update service
    void installUpdateService();

    //! Check and validate update json file
    bool checkUpdateFile(const QByteArray updateData);

    //! Find Latest version from the update JsonObject
    QString findLatestVersion(QJsonObject updateJson);

    //! Update Logs
    void updateLog(const QJsonObject updateJsonObject);

    //! Check force updates
    //! Return last force update version that in greater than current version, otherwise returns empty string
    QString findForceUpdate(const QJsonObject updateJsonObject);

    //! Update Available versions
    void updateAvailableVersions(const QJsonObject updateJsonObject);

    //! Check and prepare the system to start download process.
    void checkAndDownloadPartialUpdate(const QString installingVersion);

private:

    QJsonObject mUpdateJsonObject;

    QString mSerialNumber;

    QByteArray m_expectedUpdateChecksum;

    QStringList mAvailableVersions;

    QString mUpdateFilePath;

    //! Latest version is installable version
    //! (last force update that is greater than current version or the lastes version if no new force version exists)
    QString mLatestVersionKey;

    QString mLatestVersionDate;
    QString mLatestVersionChangeLog;
    QString mLastInstalledUpdateDate;

    int mRequiredMemory;
    int mUpdateFileSize;

    QString mRemainingDownloadTime;

    QString mUpdateDirectory;

    int mPartialUpdateProgress;

    bool mUpdateAvailable;

    bool mHasForceUpdate;

    bool mIsGetSNReceived;
    
    //! System on test mode or not
    bool mTestMode;

    QTimer mUpdateTimer;

    NUVE::cpuid_t mUID;

    //! QElapsedTimer to measure download rate.
    QElapsedTimer mElapsedTimer;
};

} // namespace NUVE

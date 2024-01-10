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

    Q_PROPERTY(QString lastInstalledUpdateDate READ lastInstalledUpdateDate NOTIFY lastInstalledUpdateDateChanged FINAL)
    Q_PROPERTY(QString latestVersion          READ latestVersion           NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionDate      READ latestVersionDate       NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionChangeLog READ latestVersionChangeLog  NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString remainingDownloadTime  READ remainingDownloadTime   NOTIFY remainingDownloadTimeChanged FINAL)
    Q_PROPERTY(QString serialNumber           READ serialNumber            NOTIFY snReady FINAL)

    Q_PROPERTY(bool updateAvailable  READ updateAvailable   NOTIFY updateAvailableChanged FINAL)

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

    //! Get technic's url and serial number
    void getQR(QString accessUid) { getSN(accessUid.toStdString()); }

    // TODO review if this, and others below, should be static
    std::string getSN(cpuid_t accessUid);

    //! Get update
    //! todo: process response packet
    //! TEMP: "022"
    void getUpdate(QString softwareVersion = "022");

    //! Send request job to web server
    void requestJob(QString type);


    //! Start the partilally update
    Q_INVOKABLE void partialUpdate();

    Q_INVOKABLE void updateAndRestart();

    //! Get update information from server
    //! notifyUser: Send notification for user when new update is available
    Q_INVOKABLE void getUpdateInformation(bool notifyUser = false);

    //! Get Contractor Information
    void getContractorInfo();

    //! Getters
    QString latestVersion();

    QString latestVersionDate();

    QString latestVersionChangeLog();

    QString remainingDownloadTime();

    QString lastInstalledUpdateDate();

    QString serialNumber();

    int partialUpdateProgress();

    bool updateAvailable();

    void setPartialUpdateProgress(int progress);



protected slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

signals:
    void snReady();

    void latestVersionChanged();
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

private:

    QString mSerialNumber;

    QByteArray m_expectedUpdateChecksum;

    QString mUpdateFilePath;

    QString mLatestVersionAddress;
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

    QTimer mTimer;

    //! QElapsedTimer to measure download rate.
    QElapsedTimer mElapsedTimer;

};

} // namespace NUVE

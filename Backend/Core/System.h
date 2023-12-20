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

    Q_PROPERTY(QString latestVersion          READ latestVersion           NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionDate      READ latestVersionDate       NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString latestVersionChangeLog READ latestVersionChangeLog  NOTIFY latestVersionChanged FINAL)
    Q_PROPERTY(QString remainingDownloadTime  READ remainingDownloadTime   NOTIFY remainingDownloadTimeChanged FINAL)

    Q_PROPERTY(int partialUpdateProgress      READ partialUpdateProgress    NOTIFY partialUpdateProgressChanged FINAL)

    QML_ELEMENT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/

    System(QObject *parent = nullptr);

    /* Public Functions (Setters and getters)
     * ****************************************************************************************/

    //! Reboot device
    void rebootDevice();

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

    //! Getters
    QString latestVersion();

    QString latestVersionDate();

    QString latestVersionChangeLog();

    QString remainingDownloadTime();

    int partialUpdateProgress();

    void setPartialUpdateProgress(int progress);


protected slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

signals:
    void snReady();

    void latestVersionChanged();
    void partialUpdateProgressChanged();
    void remainingDownloadTimeChanged();

    //! Emit when partially update is ready
    void partialUpdateReady();

    void error(QString err);

private:

    //! verify dounloaded files and prepare to set up.
    void verifyDownloadedFiles(QByteArray downloadedData);

    //! Get update information from server
    void getUpdateInformation();

    //! Check new version from file.
    //! This function call automatically.
    void checkPartialUpdate();

private:

    QString mSerialNumber;

    QByteArray m_expectedUpdateChecksum;

    QString mUpdateFilePath;

    QString mLatestVersionAddress;
    QString mLatestVersion;
    QString mLatestVersionDate;
    QString mLatestVersionChangeLog;

    QString mRemainingDownloadTime;

    int mPartialUpdateProgress;

    QTimer mTimer;

    //! QElapsedTimer to measure download rate.
    QElapsedTimer mElapsedTimer;

};

} // namespace NUVE

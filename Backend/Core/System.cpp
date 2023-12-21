#include "System.h"
#include "LogHelper.h"

#include <QProcess>
#include <QDebug>
#include <QUrl>

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
const QUrl m_domainUrl        = QUrl("http://test.hvac.z-soft.am"); // base domain
const QUrl m_engineUrl        = QUrl("/engine/index.php");          // engine
const QUrl m_updateUrl        = QUrl("/update/");                   // update

const QUrl m_updateServerUrl  = QUrl("http://fileserver.nuvehvac.com"); // New server

const QString m_getSN           = QString("getSN");
const QString m_getSystemUpdate = QString("getSystemUpdate");
const QString m_requestJob      = QString("requestJob");
const QString m_partialUpdate   = QString("partialUpdate");
const QString m_updateFromServer= QString("UpdateFromServer");

const char m_isBusyDownloader[] = "isBusyDownloader";

//! Function to calculate checksum (Md5)
inline QByteArray calculateChecksum(const QByteArray &data) {
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

NUVE::System::System(QObject *parent) :
    mUpdateAvailable (false)
{

    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &System::processNetworkReply);

    mUpdateFilePath = qApp->applicationDirPath() + "/update.json";

    connect(&mTimer, &QTimer::timeout, this, [=]() {
        getUpdateInformation();
    });

    mTimer.start(12 * 60 * 60 * 1000); // each 12 hours
    mUpdateDirectory = qApp->applicationDirPath();

#ifdef __unix__
    QProcess process;
    process.start("mkdir /mnt/update && mount /dev/mmcblk1p3 /mnt/update");
    process.waitForFinished(); // wait 30 seconds (if needed)

    // Check if the mount process executed successfully
    if (process.exitCode() == 0) {
        TRACE << "Device mounted successfully.";
        QProcess::execute("mkdir /mnt/update/latestVersion");
        mUpdateDirectory = "/mnt/update/latestVersion";

    } else {
        TRACE << "Error mounting device. Error code:" << process.exitCode();
        TRACE << "Error details:" << process.readAllStandardError();
    }

#endif

    QTimer::singleShot(0, this, [=]() {
        checkPartialUpdate();
        getUpdateInformation();
    });

}

std::string NUVE::System::getSN(cpuid_t accessUid)
{
    QJsonArray paramsArray;
    paramsArray.append(QString::fromStdString(accessUid));

    // TODO parameter retrieval from cloud, can we utilise a value/isSet tuple and push the processing to a background function?  Or are we happy with a firing off a whole bunch of requests and waiting for them to complete?
    QByteArray requestData = preparePacket("sync", m_getSN, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_getSN);

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::System::snReady, &loop, &QEventLoop::quit);
    // TODO this timeout is enough for a post request?
    // TODO the timeout needs to be defined in a paramter somewhere
    timer.start(3000);
    loop.exec();

    qDebug() << Q_FUNC_INFO << "Retrieve SN returned: " << QString::fromStdString(mSerialNumber.toStdString());

    return mSerialNumber.toStdString();
}

void NUVE::System::getUpdate(QString softwareVersion)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(softwareVersion);

    QByteArray requestData = preparePacket("sync", m_getSystemUpdate, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_getSystemUpdate);
}

void NUVE::System::getUpdateInformation() {
    // Fetch the file from web location
    QNetworkReply* reply = mNetManager->get(QNetworkRequest(m_updateServerUrl.resolved(QUrl("/update.json"))));
    TRACE << reply->url().toString();
    reply->setProperty(m_methodProperty, m_updateFromServer);
}

void NUVE::System::requestJob(QString type)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(type);

    QByteArray requestData = preparePacket("sync", m_requestJob, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_requestJob);
}

QString NUVE::System::latestVersionDate() {
    return mLatestVersionDate;
}

QString NUVE::System::latestVersionChangeLog() {
    return mLatestVersionChangeLog;
}

QString NUVE::System::latestVersion() {
    return mLatestVersion;
}

QString NUVE::System::remainingDownloadTime() {
    return mRemainingDownloadTime;
}

int NUVE::System::partialUpdateProgress() {
    return mPartialUpdateProgress;
}

void NUVE::System::setPartialUpdateProgress(int progress) {
    mPartialUpdateProgress = progress;
    emit partialUpdateProgressChanged();
}

void NUVE::System::partialUpdate() {
    if (mNetManager->property(m_isBusyDownloader).toBool()) {
        // To open progress bar.
        emit downloadStarted();
        return;
    }

    if (false) {
    QJsonObject jsonObj;

    // Extracting values from JSON
    QString hv = jsonObj["hv"].toString();
    QString require = jsonObj["require"].toString();
    QString sv = jsonObj["sv"].toString();
    QString type = jsonObj["type"].toString();
    QString filename = jsonObj["list"].toArray()[0].toObject()["filename"].toString();
    QString url = jsonObj["list"].toArray()[0].toObject()["url"].toString();

    // Construct web file URL
    QString webFile = m_domainUrl.toString() + m_updateUrl.toString() +
                       hv + require + sv + type + "/" + filename;
    }

    emit downloadStarted();

    // Fetch the file from web location
    QNetworkReply* reply = mNetManager->get(QNetworkRequest(m_updateServerUrl.resolved(QUrl(mLatestVersionAddress))));
    reply->setProperty(m_methodProperty, m_partialUpdate);
    mNetManager->setProperty(m_isBusyDownloader, true);

    setPartialUpdateProgress(0);

    if (mElapsedTimer.isValid())
        mElapsedTimer.invalidate();

    mElapsedTimer.start();
    connect(reply, &QNetworkReply::downloadProgress, this, [=] (qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal == 0)
            return;

        double secTime = mElapsedTimer.elapsed() / 1000;
        double rate = bytesReceived / (secTime > 0 ? secTime : 1.0);
        auto remain = bytesTotal - bytesReceived;
        int remainTime = rate < 0.001 ? 1000000 : qRound(remain / rate);

        QString unit = remainTime < 60 ? "second" : "minute";

        remainTime = remainTime < 60 ? remainTime : qRound(remainTime / 60.0);

        if (remainTime > 1)
            unit += "s";

        mRemainingDownloadTime = QString("About %1 %2 remaining").arg(QString::number(remainTime), unit);
        emit remainingDownloadTimeChanged();

        int percentage = bytesReceived * 100 / bytesTotal;
        setPartialUpdateProgress(percentage);
    });

}

void NUVE::System::updateAndRestart()
{
    // Define source and destination directories
    QString destDir = qApp->applicationDirPath();

    // Run the shell script with source and destination arguments
    // - copy files from source to destination folder
    // - run the app
    QString scriptPath = destDir + "/update.sh";
    QStringList arguments;
    arguments << scriptPath << mUpdateDirectory << destDir;

    QProcess::startDetached("/bin/bash", arguments);
}


// Checksum verification after download
void NUVE::System:: verifyDownloadedFiles(QByteArray downloadedData) {
    QByteArray downloadedChecksum = calculateChecksum(downloadedData);

    if (downloadedChecksum == m_expectedUpdateChecksum) {

        // Checksums match - downloaded app is valid
        // Save the downloaded data
        QFile file(mUpdateDirectory + "/update.gz");
        if (!file.open(QIODevice::WriteOnly)) {
            TRACE << "Unable to open file for writing";
            emit error("Unable to open file for writing");
            return;
        }
        file.write(downloadedData);
        file.close();

        emit partialUpdateReady();

    } else {
        // Checksums don't match - downloaded app might be corrupted
        TRACE << "Checksums don't match - downloaded app might be corrupted";

        emit error("Checksums don't match - downloaded app might be corrupted");
    }
}

void NUVE::System::processNetworkReply(QNetworkReply *netReply)
{
    NetworkWorker::processNetworkReply(netReply);

    if (netReply->error() != QNetworkReply::NoError) {
        if (netReply->operation() == QNetworkAccessManager::GetOperation) {

            if (netReply->property(m_methodProperty).toString() == m_updateFromServer) {
                getUpdateInformation();

            } else if (netReply->property(m_methodProperty).toString() == m_partialUpdate) {
                mNetManager->setProperty(m_isBusyDownloader, false);
                emit error("Download error: " + netReply->errorString());
            }
        }

        return;
    }

    QByteArray data = netReply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonObject obj = doc.object();

    switch (netReply->operation()) {
    case QNetworkAccessManager::PostOperation: {

        if (netReply->property(m_methodProperty).toString() == m_getSN) {
            QJsonArray resultArray = obj.value("result").toObject().value("result").toArray();
            qDebug() << Q_FUNC_INFO << __LINE__ << resultArray;

            if (resultArray.count() > 0) {
                mSerialNumber = resultArray.first().toString();
                Q_EMIT snReady();
            }

        } else if (netReply->property(m_methodProperty).toString() == m_getSystemUpdate) {
            auto resultObj = obj.value("result").toObject().value("result").toObject();
            auto isRequire = (resultObj.value("require").toString() == "r");
            auto updateType = resultObj.value("type").toString();

            if (isRequire) {
                if (updateType == "f") {
                    qDebug() << Q_FUNC_INFO << __LINE__ << "The system requires a full update.";

                } else {
                    qDebug() << Q_FUNC_INFO << __LINE__ << "The system requires a partial update.";

                    partialUpdate();

                }
            }
        }

    } break;
    case QNetworkAccessManager::GetOperation: {

        // Partial update (download process) finished.
        if (netReply->property(m_methodProperty).toString() == m_partialUpdate) {

            // Check data and prepare to set up.
            verifyDownloadedFiles(data);
            mNetManager->setProperty(m_isBusyDownloader, false);

        } else if (netReply->property(m_methodProperty).toString() == m_updateFromServer) { // Partial update (download process) finished.

            TRACE << mUpdateFilePath;
            // Save the downloaded data
            QFile file(mUpdateFilePath);
            if (!file.open(QIODevice::WriteOnly)) {
                TRACE << "Unable to open file for writing";
                emit error("Unable to open file for writing");
                return;
            }
            TRACE << data;
            file.write(data);
            file.close();

            checkPartialUpdate();
        }

    } break;

    default:

        break;
    }
}

void NUVE::System::checkPartialUpdate() {

    // Save the downloaded data
    QFile file(mUpdateFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        TRACE << "Unable to open file for reading";
        return;
    }

    auto updateJsonObject = QJsonDocument::fromJson(file.readAll()).object();

    file .close();

    // Update version information
    mLatestVersion = updateJsonObject.value("LatestVersion").toString();

    // Check version (app and latest)
    auto currentVersion = qApp->applicationVersion();
    if (mLatestVersion != currentVersion) {
        auto appVersionList = currentVersion.split(".");
        auto latestVersion = mLatestVersion.split(".");

        if (appVersionList.count() > 2 && latestVersion.count() > 2) {

            auto appVersionMajor = appVersionList.first().toInt();
            auto latestVersionMajor = latestVersion.first().toInt();

            auto appVersionMinor = appVersionList[1].toInt();
            auto latestVersionMinor = latestVersion[1].toInt();

            auto appVersionPatch = appVersionList[2].toInt();
            auto latestVersionPatch = latestVersion[2].toInt();

            bool isUpdateAvailable = latestVersionMajor > appVersionMajor;


            if (latestVersionMajor == appVersionMajor) {
                isUpdateAvailable = latestVersionMinor > appVersionMinor;

                if (latestVersionMinor == appVersionMinor)
                    isUpdateAvailable = latestVersionPatch > appVersionPatch;
            }
            setUpdateAvailable(isUpdateAvailable);
        }

    }

    auto latestVersionObj = updateJsonObject.value(mLatestVersion).toObject();
    mLatestVersionDate = latestVersionObj.value("releaseDate").toString();
    mLatestVersionChangeLog = latestVersionObj.value("changeLog").toString();
    mLatestVersionAddress = latestVersionObj.value("address").toString();

    m_expectedUpdateChecksum = QByteArray::fromHex(latestVersionObj.value("checkSum").toString().toLatin1());

    emit latestVersionChanged();
}

void NUVE::System::rebootDevice()
{
        QProcess process;
        QString command = "sudo reboot";

        process.start(command);
        process.waitForFinished();

        int exitCode = process.exitCode();
        QByteArray result = process.readAllStandardOutput();
        QByteArray error = process.readAllStandardError();

        qDebug() << "Exit Code:" << exitCode;
        qDebug() << "Standard Output:" << result;
        qDebug() << "Standard Error:" << error;
}

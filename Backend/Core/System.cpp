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


const QString m_updateService   = QString("/etc/systemd/system/appStherm-update.service");

const char m_isBusyDownloader[] = "isBusyDownloader";

constexpr char m_notifyUserProperty[] = "notifyUser";

/* ************************************************************************************************
 * Update Json Keys
 * ************************************************************************************************/
const QString m_LatestVersion   = QString("LatestVersion");
const QString m_ReleaseDate     = QString("ReleaseDate");
const QString m_ChangeLog       = QString("ChangeLog");
const QString m_Address         = QString("Address");
const QString m_RequiredMemory  = QString("RequiredMemory");
const QString m_CurrentFileSize = QString("CurrentFileSize");
const QString m_CheckSum        = QString("CheckSum");

const QString m_InstalledUpdateDateSetting = QString("Stherm/UpdateDate");

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
        getUpdateInformation(true);
    });

    mTimer.start(12 * 60 * 60 * 1000); // each 12 hours
    mUpdateDirectory = qApp->applicationDirPath();

    // Install update service
    installUpdateService();

    mountUpdateDirectory();

    QSettings setting;
    mLastInstalledUpdateDate = setting.value(m_InstalledUpdateDateSetting).toString();

    QTimer::singleShot(5 * 60 * 1000, this, [=]() {
        checkPartialUpdate(true);
        getUpdateInformation(true);
    });

}

NUVE::System::~System()
{
    delete mNetManager;
}

void  NUVE::System::installUpdateService()
{
#ifdef __unix__
    QFile updateFileSH("/usr/local/bin/update.sh");
    if (updateFileSH.exists())
        updateFileSH.remove("/usr/local/bin/update.sh");

    TRACE << "update.sh file updated: " << QFile::copy(":/Stherm/update.sh", "/usr/local/bin/update.sh");

    QProcess::execute("/bin/bash", {"-c", "chmod +x /usr/local/bin/update.sh"});

    QFile updateServiceFile(m_updateService);

    QString serviceContent = "[Unit]\n"
                             "Description=Nuve Smart HVAC system update service\n"
                             "[Service]\n"
                             "Type=simple\n"
                             "ExecStart=/bin/bash -c \"/usr/local/bin/update.sh\"\n"
                             "Restart=on-failure\n"
                             "[Install]\n"
                             "WantedBy=multi-user.target";

    bool neetToUpdateService = true;

    // Check the service
    if (updateServiceFile.exists()) {
        if (updateServiceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            neetToUpdateService = updateServiceFile.readAll() != serviceContent.toUtf8();
            updateServiceFile.close();

        }
    }

    if (neetToUpdateService && updateServiceFile.open(QIODevice::WriteOnly | QIODevice::Text)) {

        updateServiceFile.write(serviceContent.toUtf8());

        updateServiceFile.close();

        // Reload systemd to read the updated service files
        // QProcess::execute("/bin/bash", {"-c", "systemctl daemon-reload"});

        TRACE << "The update service successfully installed.";

    } else if (!neetToUpdateService) {
        TRACE << "The service is already installed..";

    } else {
        TRACE << "Unable to install the update service.";
    }

#endif
}

void  NUVE::System::mountUpdateDirectory()
{
#ifdef __unix__
    int exitCode = QProcess::execute("/bin/bash", {"-c", "mkdir /mnt/update; mount /dev/mmcblk1p3 /mnt/update"});
    // Check if the mount process executed successfully

    TRACE << "Device mounted successfully." << QProcess::execute("/bin/bash", {"-c", "mkdir /mnt/update/latestVersion"}) << exitCode;
    mUpdateDirectory = "/mnt/update/latestVersion";
#endif

}

void NUVE::System::setUpdateAvailable(bool updateAvailable) {
    if (mUpdateAvailable == updateAvailable)
        return;

    mUpdateAvailable = updateAvailable;
    emit updateAvailableChanged();
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

void NUVE::System::getUpdateInformation(bool notifyUser) {
    // Fetch the file from web location
    QNetworkReply* reply = mNetManager->get(QNetworkRequest(m_updateServerUrl.resolved(QUrl("/update.json"))));
    TRACE << reply->url().toString();
    reply->setProperty(m_methodProperty, m_updateFromServer);
    reply->setProperty(m_notifyUserProperty, notifyUser);
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
    return mLatestVersionKey;
}

QString NUVE::System::remainingDownloadTime() {
    return mRemainingDownloadTime;
}

QString NUVE::System::lastInstalledUpdateDate()
{
    return mLastInstalledUpdateDate;
}

int NUVE::System::partialUpdateProgress() {
    return mPartialUpdateProgress;
}

bool NUVE::System::updateAvailable() {
    return mUpdateAvailable;
}

void NUVE::System::setPartialUpdateProgress(int progress) {
    mPartialUpdateProgress = progress;
    emit partialUpdateProgressChanged();
}

void NUVE::System::partialUpdate() {

    // Check
    QStorageInfo storageInfo (mUpdateDirectory);

    if (!storageInfo.isValid()) {
        mountUpdateDirectory();
    }

    if (!storageInfo.isValid() || !storageInfo.isReady()) {
        emit error("The update directory is not ready.");
        return;
    }


    // Check update file
    QFile file(mUpdateDirectory + "/update.zip");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {

        auto downloadedData = file.readAll();
        file.close();

        if (verifyDownloadedFiles(downloadedData, false))
            return;
        else
            TRACE << "The file update needs to be redownloaded.";
    }

    if (storageInfo.bytesFree() < mUpdateFileSize) {

        QDir dir(mUpdateDirectory);
        // Removes the directory, including all its contents.
        dir.removeRecursively();

        // Create the latestVersion directory
#ifdef __unix__
        TRACE << "Device mounted successfully." << QProcess::execute("/bin/bash", {"-c", "mkdir /mnt/update/latestVersion"});
#endif

        if (storageInfo.bytesFree() < mUpdateFileSize) {
            emit error(QString("The update directory has no memory. Required memory is %0, and available memory is %1.")
                           .arg(QString::number(mUpdateFileSize), QString::number(storageInfo.bytesFree())));
            return;
        }
    }

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
    //    // Define source and destination directories
    //    QString destDir = qApp->applicationDirPath();


    //    // Run the shell script with source and destination arguments
    //    // - copy files from source to destination folder
    //    // - run the app
    //    QString scriptPath = destDir + "/update.sh";
    //    QStringList arguments;
    //    arguments << scriptPath << mUpdateDirectory << destDir;

    // Use to unzip the downloaded files.
    QStorageInfo updateStorageInfo (mUpdateDirectory);

    if (updateStorageInfo.bytesFree() < mUpdateFileSize) {

        QString err = QString("The update directory has no memory for intallation.\nRequired memory is %0 bytes, and available memory is %1 bytes.")
                          .arg(QString::number(mUpdateFileSize), QString::number(updateStorageInfo.bytesFree()));
        emit error(err);
        TRACE << err;

        return;
    }

    QStorageInfo installStorageInfo (qApp->applicationDirPath());
    QFileInfo appInfo(qApp->applicationFilePath());

    if ((installStorageInfo.bytesFree() + appInfo.size()) < mRequiredMemory) {
        QString err = QString("The update directory has no memory for intallation.\nRequired memory is %0 bytes, and available memory is %1 bytes.")
                          .arg(QString::number(mRequiredMemory), QString::number(installStorageInfo.bytesFree()));
        emit error(err);
        TRACE << err;

        return;
    }

    TRACE << "starting update" ;

#ifdef __unix__
    // It's incorrect if the update process failed,
    // but in that case, the update is available and
    // this property remains hidden.
    mLastInstalledUpdateDate = QDate::currentDate().toString("dd/MM/yyyy");
    QSettings setting;
    setting.setValue(m_InstalledUpdateDateSetting, mLastInstalledUpdateDate);

    emit lastInstalledUpdateDateChanged();

    emit systemUpdating();

    installUpdateService();

    int exitCode = QProcess::execute("/bin/bash", {"-c", "systemctl enable appStherm-update.service; systemctl start appStherm-update.service"});
    TRACE << exitCode;
#endif


}


// Checksum verification after download
bool NUVE::System:: verifyDownloadedFiles(QByteArray downloadedData, bool withWrite) {
    QByteArray downloadedChecksum = calculateChecksum(downloadedData);

    if (downloadedChecksum == m_expectedUpdateChecksum) {

        // Checksums match - downloaded app is valid
        // Save the downloaded data
        if (withWrite) {
            QFile file(mUpdateDirectory + "/update.zip");
            if (!file.open(QIODevice::WriteOnly)) {
                emit error("Unable to open file for writing in " + mUpdateDirectory);
                return false;
            }
            file.write(downloadedData);
            file.close();
        }

        emit partialUpdateReady();

        return true;

    } else if (withWrite) {
        // Checksums don't match - downloaded app might be corrupted
        TRACE << "Checksums don't match - downloaded app might be corrupted";

        emit error("Checksums don't match - downloaded app might be corrupted");
    }

    return false;
}

void NUVE::System::processNetworkReply(QNetworkReply *netReply)
{
    NetworkWorker::processNetworkReply(netReply);

    if (netReply->error() != QNetworkReply::NoError) {
        if (netReply->operation() == QNetworkAccessManager::GetOperation) {

            if (netReply->property(m_methodProperty).toString() == m_updateFromServer) {
                qWarning() << "Unable to download update.json file: " << netReply->errorString();
                emit alert("Unable to download update information, Please check your internet connection: " + netReply->errorString());

            } else if (netReply->property(m_methodProperty).toString() == m_partialUpdate) {
                mNetManager->setProperty(m_isBusyDownloader, false);
                emit error("Download error: " + netReply->errorString());
            }
        }

        netReply->deleteLater();
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
            if (checkUpdateFile(data)) {

                QFile file(mUpdateFilePath);
                if (!file.open(QIODevice::WriteOnly)) {
                    TRACE << "Unable to open file for writing";
                    emit error("Unable to open file for writing");
                    break;;
                }
                TRACE << data;

                file.write(data);

                file.close();
            } else {
                emit alert("The update information fetched corrupted, Contact Administrator!");
            }

            // Check the last saved update.json file
            checkPartialUpdate(netReply->property(m_notifyUserProperty).toBool());
        }

    } break;

    default:

        break;
    }

    netReply->deleteLater();
}

bool NUVE::System::checkUpdateFile(const QByteArray updateData) {
    auto updateDoc = QJsonDocument::fromJson(updateData);
    if (updateDoc.isNull()) {
        qWarning() << "The update information has invalid format (server side).";
        return false;
    }

    auto updateJson = updateDoc.object();

    if (updateJson.contains(m_LatestVersion)) {
        auto latestVersion = updateJson.value(m_LatestVersion).toString();
        if (latestVersion.split(".").count() == 3) {

            if (!updateJson.contains(latestVersion)) {
                qWarning() << "The 'LatestVersion' value (" << latestVersion << ") is not found or has invalid format in the Update file (server side).";
                return false;
            }

            QStringList jsonKeys;
            jsonKeys << m_ReleaseDate
                     << m_ChangeLog
                     << m_Address
                     << m_RequiredMemory
                     << m_CurrentFileSize
                     << m_CheckSum;

            auto latestVersionObj = updateJson.value(latestVersion).toObject();
            if (latestVersionObj.isEmpty()) {
                qWarning() << "The 'LatestVersion' value (" << latestVersion << ") is empty in the Update file (server side).";
                return false;
            }

            foreach (auto key, jsonKeys) {
                auto value = latestVersionObj.value(key);
                if (value.isUndefined() || value.type() == QJsonValue::Null) {
                    qWarning() << "The key (" << key << ") not found in the 'LatestVersion' value (" << latestVersion << ") (server side).";
                    return false;
                }

                if (value.isString() && value.toString().isEmpty()) {
                    qWarning() << "The key (" << key << ") is empty in the 'LatestVersion' value (" << latestVersion << ") (server side).";
                    return false;

                } else if (value.isDouble() && (value.toDouble(-100) == -100)) {
                    qWarning() << "The key (" << key << ") is empty in the 'LatestVersion' value (" << latestVersion << ") (server side).";
                    return false;
                }
            }

        } else {
            qWarning() << "The 'LatestVersion' value (" << latestVersion << ") is incorrect in the Update file (server side).";
            return false;
        }

    } else {
        qWarning() << "The 'LatestVersion' key is not present in the Update file (server side).";
        return false;
    }

    return true;
}

void NUVE::System::checkPartialUpdate(bool notifyUser) {

    // Save the downloaded data
    QFile file(mUpdateFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        TRACE << "Unable to open file for reading";
        return;
    }

    auto updateJsonObject = QJsonDocument::fromJson(file.readAll()).object();

    file .close();

    // Update version information
    mLatestVersionKey = updateJsonObject.value("LatestVersion").toString();

    // Check version (app and latest)
    auto currentVersion = qApp->applicationVersion();
    if (mLatestVersionKey != currentVersion) {
        auto appVersionList = currentVersion.split(".");
        auto latestVersion = mLatestVersionKey.split(".");

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

            if (notifyUser)
                emit notifyNewUpdateAvailable();

        } else {
            qWarning() << "The version format is incorrect (major.minor.patch)" << mLatestVersionKey;
        }

    }

    auto latestVersionObj = updateJsonObject.value(mLatestVersionKey).toObject();
    mLatestVersionDate = latestVersionObj.value(m_ReleaseDate).toString();
    mLatestVersionChangeLog = latestVersionObj.value(m_ChangeLog).toString();
    mLatestVersionAddress = latestVersionObj.value(m_Address).toString();
    mRequiredMemory = latestVersionObj.value(m_RequiredMemory).toInt();
    mUpdateFileSize = latestVersionObj.value(m_CurrentFileSize).toInt();

    m_expectedUpdateChecksum = QByteArray::fromHex(latestVersionObj.value(m_CheckSum).toString().toLatin1());

    if (mLastInstalledUpdateDate.isEmpty())
        mLastInstalledUpdateDate = mLatestVersionDate;

    emit latestVersionChanged();
}

void NUVE::System::rebootDevice()
{
#ifdef __unix__
    QProcess process;
    QString command = "reboot";

    process.start(command);
    process.waitForFinished();

    int exitCode = process.exitCode();
    QByteArray result = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();

    qDebug() << "Exit Code:" << exitCode;
    qDebug() << "Standard Output:" << result;
    qDebug() << "Standard Error:" << error;

    #endif
}

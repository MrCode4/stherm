#include "System.h"
#include "LogHelper.h"
#include "UtilityHelper.h"

#include <QProcess>
#include <QDebug>
#include <QUrl>

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
const QUrl m_updateServerUrl  = QUrl("http://fileserver.nuvehvac.com"); // New server
const QString m_logUsername = "uploadtemp";
const QString m_logPassword = "oDhjPTDJYkUOvM9";
const QString m_logServerAddress = "fileserver.nuvehvac.com";
const QString m_logPath = "/opt/logs/";

const QString m_partialUpdate   = QString("partialUpdate");
const QString m_backdoorUpdate   = QString("backdoorUpdate");
const QString m_updateFromServer= QString("UpdateFromServer");
const QString m_backdoorFromServer = QString("BackdoorFromServer");

const QString m_checkInternetConnection = QString("checkInternetConnection");

const QString m_updateService   = QString("/etc/systemd/system/appStherm-update.service");

const char m_isBusyDownloader[] = "isBusyDownloader";
const char m_isResetVersion[]   = "isResetVersion";

constexpr char m_notifyUserProperty[] = "notifyUser";

const int m_timeout = 100000; // 100 seconds

/* ************************************************************************************************
 * Update Json Keys
 * ************************************************************************************************/
const QString m_ReleaseDate     = QString("ReleaseDate");
const QString m_ChangeLog       = QString("ChangeLog");
const QString m_Address         = QString("Address");
const QString m_RequiredMemory  = QString("RequiredMemory");
const QString m_CurrentFileSize = QString("CurrentFileSize");
const QString m_CheckSum        = QString("CheckSum");
const QString m_Staging         = QString("Staging");
const QString m_ForceUpdate     = QString("ForceUpdate");

const QString m_InstalledUpdateDateSetting = QString("Stherm/UpdateDate");
const QString m_SerialNumberSetting        = QString("Stherm/SerialNumber");
const QString m_IsManualUpdateSetting        = QString("Stherm/IsManualUpdate");

const QString m_updateOnStartKey = "updateSequenceOnStart";

//! Function to calculate checksum (Md5)
inline QByteArray calculateChecksum(const QByteArray &data) {
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

//! isVersionNewer
//! return true when version1 > version2
//! return false when version1 <= version2
bool isVersionNewer(const QString& version1, const QString& version2) {
    QStringList parts1 = version1.split('.');
    QStringList parts2 = version2.split('.');

    // Compare each component numerically
    for (int i = 0; i < qMax(parts1.size(), parts2.size()); i++) {
        int part1 = (i < parts1.size()) ? parts1[i].toInt() : 0;
        int part2 = (i < parts2.size()) ? parts2[i].toInt() : 0;

        if (part1 > part2) {
            return true;  // version1 is greater than version2

        } else if (part1 < part2) {
            return false;  // version1 is less than version2
        }
    }

    return false;  // versions are equal
}

NUVE::System::System(NUVE::Sync *sync, QObject *parent) : NetworkWorker(parent),
    mSync(sync),
    mUpdateAvailable (false),
    mHasForceUpdate(false),
    mIsInitialSetup(false),
    mTestMode(false)
{

    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &System::processNetworkReply);

    mUpdateFilePath = qApp->applicationDirPath() + "/updateInfo.json";

    connect(&mFetchActiveTimer, &QTimer::timeout, this, [=]() {
        setCanFetchServer(true);
    });

    connect(&mUpdateTimer, &QTimer::timeout, this, [=]() {
        getUpdateInformation(true);
    });

    mUpdateTimer.setInterval(12 * 60 * 60 * 1000); // each 12 hours
    mUpdateDirectory = qApp->applicationDirPath();

    // Install update service
    installUpdateService();

    mountUpdateDirectory();
    mountRecoveryDirectory();
    mountNRF_FW_Directory();
    installUpdate_NRF_FW_Service();

    if (!mountDirectory("/mnt/log", "/mnt/log/log"))
        qWarning() << "unable to create logs folder";

    QSettings setting;
    mLastInstalledUpdateDate = setting.value(m_InstalledUpdateDateSetting).toString();
    mIsManualUpdate          = setting.value(m_IsManualUpdateSetting, false).toBool();
    mStartedWithManualUpdate = mIsManualUpdate;

    // reformat if it was saved with old format
    auto oldFormatDate = QDate::fromString(mLastInstalledUpdateDate, "dd/MM/yyyy");
    if (oldFormatDate.isValid())
        mLastInstalledUpdateDate = oldFormatDate.toString("dd MMM yyyy");

    connect(mSync, &NUVE::Sync::snReady, this, &NUVE::System::onSnReady);
    connect(mSync, &NUVE::Sync::alert, this, &NUVE::System::alert);
    connect(mSync, &NUVE::Sync::settingsReady, this, &NUVE::System::settingsReady);
    connect(mSync, &NUVE::Sync::pushFailed, this, &NUVE::System::pushFailed);
    connect(mSync, &NUVE::Sync::pushSuccess, this, [this]() {
        mFetchActiveTimer.start(10 * 1000); // can fetch, 10 seconds after a successful push
    });

    connect(this, &NUVE::System::systemUpdating, this, [this](){
        QSettings settings;
        settings.setValue("m_updateOnStartKey", true);
    });

    // Check: The downloader has been open for more than 30 seconds and has not received any bytes
    downloaderTimer.setTimerType(Qt::PreciseTimer);
    downloaderTimer.setSingleShot(false);

    connect(&mLogSender, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        qWarning() << "process has encountered an error:" << error << mLogSender.readAllStandardError();
    });
    connect(&mLogSender, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            qWarning() << "process did not exit cleanly" << exitCode << exitStatus << mLogSender.readAllStandardError()
                       << mLogSender.readAllStandardOutput();
            return;
        }
        auto initialized = mLogSender.property("initialized");
        if (!initialized.isValid() || !initialized.toBool()){
            TRACE << "Folder created in server successfully";
            mLogSender.setProperty("initialized", true);
        }
    });

    if (!serialNumber().isEmpty())
        onSnReady();
}

NUVE::System::~System()
{
    delete mNetManager;
}

bool NUVE::System::installUpdateService()
{
#ifdef __unix__
    QFile updateFileSH("/usr/local/bin/update.sh");
    if (updateFileSH.exists())
        updateFileSH.remove("/usr/local/bin/update.sh");

    QFile copyFile(":/Stherm/update.sh");
    if (!copyFile.copy("/usr/local/bin/update.sh")) {
        TRACE << "update.sh file did not updated: " << copyFile.errorString();
        TRACE << qApp->applicationDirPath();

        return false;
    }

    auto exitCode = QProcess::execute("/bin/bash", {"-c", "chmod +x /usr/local/bin/update.sh"});
    if (exitCode == -1 || exitCode == -2)
        return false;

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

        return false;
    }

    // Disable the appStherm-update.service
    exitCode = QProcess::execute("/bin/bash", {"-c", "systemctl disable appStherm-update.service;"});
    if (exitCode == -1 || exitCode == -2)
        return false;

#endif
    return true;
}

bool NUVE::System::installUpdate_NRF_FW_Service()
{
#ifdef __unix__
    QFile updateFileSH("/usr/local/bin/update_fw_nrf_seamless.sh");
    if (updateFileSH.exists())
        updateFileSH.remove("/usr/local/bin/update_fw_nrf_seamless.sh");

    QFile copyFile(":/Stherm/update_fw_nrf_seamless.sh");
    if (!copyFile.copy("/usr/local/bin/update_fw_nrf_seamless.sh")) {
        TRACE << "update_fw_nrf_seamless.sh file did not updated: " << copyFile.errorString();
        TRACE << qApp->applicationDirPath();

        return false;
    }

    QFile updateFileZip("/mnt/update/nrf_fw/update.zip");
    if (updateFileZip.exists())
        updateFileZip.remove("/mnt/update/nrf_fw/update.zip");


    QFile copyFileZip(":/Stherm/nrf_fw_update.zip");
    if (!copyFileZip.copy("/mnt/update/nrf_fw/update.zip")) {
        TRACE << "udpate.zip file did not updated: " << copyFileZip.errorString();
        TRACE << qApp->applicationDirPath();

        return false;
    }

    auto exitCode = QProcess::execute("/bin/bash", {"-c", "chmod +x /usr/local/bin/update_fw_nrf_seamless.sh"});
    if (exitCode == -1 || exitCode == -2)
        return false;
#endif
    return true;
}
bool  NUVE::System::mountUpdateDirectory()
{
    if (mountDirectory("/mnt/update", "/mnt/update/latestVersion")) {

#ifdef __unix__
        mUpdateDirectory = "/mnt/update/latestVersion";
#endif
        return true;
    }

    return false;
}

bool  NUVE::System::mountRecoveryDirectory()
{
    if (mountDirectory("/mnt/recovery", "/mnt/recovery/recovery")) {
#ifdef __unix__
        mRecoveryDirectory = "/mnt/recovery/recovery";
#endif
        return true;
    }

    return false;
}

bool NUVE::System::mountNRF_FW_Directory()
{
    if (mountDirectory("/mnt/update", "/mnt/update/nrf_fw")) {
        TRACE << "nrf fw mounted to /mnt/recovery/recovery";
        return true;
    }
    TRACE << "nrf fw did not mount";
    return false;
}

QString NUVE::System::kernelBuildVersion()
{
    QProcess process;

    // Set the command to "uname" with the argument "-v"
    process.start("uname", QStringList() << "-v");

    // Wait for the process to finish
    if (process.waitForFinished()) {
        // Read the output of the process
        QByteArray result = process.readAllStandardOutput();
        QString response(result);

        TRACE << "Response:" << response;

        return response;

    } else {
        // Handle errors
        TRACE << "Error:" << process.errorString();
    }

    return QString();
}

QString NUVE::System::rootfsBuildTimestamp()
{
    QFile rootfsFile("/etc/timestamp");

    if (rootfsFile.open(QIODevice::ReadOnly)) {
        return rootfsFile.readAll();
    }

    return QString();
}

bool NUVE::System::findBackdoorVersion(const QString fileName)
{
    if (fileName.isEmpty())
        return false;

    // Read the downloaded data
    QFile file(qApp->applicationDirPath() + "/files_info.json");

    if (!file.exists())
        getBackdoorInformation();

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        TRACE << "Unable to open file (files_info.json) for reading";
        return false;
    }

    auto jsonData = QJsonDocument::fromJson(file.readAll()).object();

    if (jsonData.keys().contains(fileName)) {
        auto value = jsonData.value(fileName);
        if (value.isObject()) {
            auto valueObj = value.toObject();
            mBackdoorFileName = fileName;
            mExpectedBackdoorChecksum = QByteArray::fromHex(valueObj.value(m_CheckSum).toString().toLatin1());
            mBackdoorUpdateFileSize = valueObj.value(m_CurrentFileSize).toInt(-1);
            mBackdoorRequiredMemory = valueObj.value(m_RequiredMemory).toInt(-1);

            mBackdoorLog = valueObj.value(m_ChangeLog).toString();
            emit backdoorLogChanged();

            return !mExpectedBackdoorChecksum.isEmpty() &&
                   mBackdoorUpdateFileSize != -1 &&
                   mBackdoorRequiredMemory != -1;
        }
    }

    return false;
}

void NUVE::System::sendLog()
{
    if (mLogSender.state() != QProcess::NotRunning || mLogRemoteFolder.isEmpty()){
        QString error("Previous session is in progress, please try again later.");
        qWarning() << error << "State is :" << mLogSender.state() << "mLogRemoteFolder" << mLogRemoteFolder;
        emit alert(error);
        return;
    }

    QString filename = "/mnt/log/log/" + QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmss") + ".log";

    TRACE << "generating log file in " << filename;

    // Create log
    auto exitCode = QProcess::execute("/bin/bash", {"-c", "journalctl -u appStherm > " + filename});
    if (exitCode < 0)
    {
        qWarning() << "Unable to create log file";
        return;
    }


    auto initialized = mLogSender.property("initialized");
    if (!initialized.isValid() || !initialized.toBool()){
        qWarning() << "Folder was not created successfully, trying again...";
        createLogDirectoryOnServer();
        emit alert("Server is not ready, try again later!");
        return;
    }

    // Copy file to remote path, should be execute detached but we should prevent a new one before current one finishes
    QString copyFile = QString("/usr/local/bin/sshpass -p '%1' scp  -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2 %3@%4:%5").
                       arg(m_logPassword, filename, m_logUsername, m_logServerAddress, mLogRemoteFolder);
    TRACE << "sending log to server " << mLogRemoteFolder;
    mLogSender.start("/bin/bash", {"-c", copyFile});
}

bool NUVE::System::mountDirectory(const QString targetDirectory, const QString targetFolder)
{
#ifdef __unix__
    int exitCode = QProcess::execute("/bin/bash", {"-c", "mkdir "+ targetDirectory + "; mount /dev/mmcblk1p3 " + targetDirectory });
    if (exitCode < 0)
        return false;

    TRACE << "Device mounted successfully." << exitCode;

    exitCode = QProcess::execute("/bin/bash", {"-c", "mkdir " + targetFolder});
    TRACE << exitCode;
    if (exitCode < 0)
        return false;
#endif

    return true;
}

void NUVE::System::setUpdateAvailable(bool updateAvailable) {
    if (mUpdateAvailable == updateAvailable)
        return;

    mUpdateAvailable = updateAvailable;
    emit updateAvailableChanged();
}

std::pair<std::string, bool> NUVE::System::getSN(NUVE::cpuid_t accessUid)
{
    auto response = mSync->getSN(accessUid);
    if (response.second)
        setUID(accessUid);
    return response;
}

bool NUVE::System::getUpdate(QString softwareVersion)
{
    if (mCanFetchServer) {
        return mSync->getSettings();
    }

    return false;
}

void NUVE::System::getUpdateInformation(bool notifyUser) {
    // Fetch the file from web location
    QNetworkReply* reply = mNetManager->get(QNetworkRequest(m_updateServerUrl.resolved(QUrl("/updateInfo.json"))));
    TRACE << reply->url().toString();
    reply->setProperty(m_methodProperty, m_updateFromServer);
    reply->setProperty(m_notifyUserProperty, mIsManualUpdate ? false : notifyUser);
}

void NUVE::System::getBackdoorInformation() {
    // Fetch the backdoor file from web location
    QNetworkReply* reply = mNetManager->get(QNetworkRequest(m_updateServerUrl.resolved(QUrl("/manual_update/files_info.json"))));
    TRACE << "backdoor information - URL: " << reply->url().toString();
    reply->setProperty(m_methodProperty, m_backdoorFromServer);
}

void NUVE::System::wifiConnected(bool hasInternet) {
    if (!hasInternet) {
        mUpdateTimer.stop();
        return;
    }

    mUpdateTimer.start();

    // When is initial setup, skip update Information as we want to wait until its complete!
    if (!mIsInitialSetup)
        getUpdateInformation(true);

    getBackdoorInformation();
}

void NUVE::System::pushSettingsToServer(const QVariantMap &settings, bool hasSettingsChanged)
{
    // if timer running and hasSettingsChanged stop to prevent canFetchServer issues
    if (mFetchActiveTimer.isActive() && hasSettingsChanged) {
        mFetchActiveTimer.stop();
    }

    // set when settings changed or no timer is active! otherwise let the timer do the job!
    if (!mFetchActiveTimer.isActive() || hasSettingsChanged){
        setCanFetchServer(!hasSettingsChanged);
    }

    mSync->pushSettingsToServer(settings);
}

void NUVE::System::exitManualMode()
{
    // Manual mode is false
    if (!mStartedWithManualUpdate) {
        return;
    }

    mIsManualUpdate = false;

    checkPartialUpdate(false, true);
}

void NUVE::System::setCanFetchServer(bool canFetch)
{
    if (mCanFetchServer == canFetch)
        return;

    mCanFetchServer = canFetch;
    emit canFetchServerChanged();
}

bool NUVE::System::canFetchServer()
{
    return mCanFetchServer;
}

QVariantMap NUVE::System::getContractorInfo() {
    return mSync->getContractorInfo();
}

QStringList NUVE::System::availableVersions()
{
    std::sort(mAvailableVersions.begin(), mAvailableVersions.end(), isVersionNewer);
    return mAvailableVersions;
}

void NUVE::System::requestJob(QString type)
{
    mSync->requestJob(type);
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

QString NUVE::System::serialNumber()
{
    return QString::fromStdString(mSync->getSN().first);
}

QString NUVE::System::backdoorLog()
{
    return mBackdoorLog;
}

int NUVE::System::partialUpdateProgress() {
    return mPartialUpdateProgress;
}

bool NUVE::System::updateAvailable() {
    return mUpdateAvailable;
}

bool NUVE::System::testMode() {
    return mTestMode;
}

bool NUVE::System::isManualMode() {
    return mStartedWithManualUpdate;
}

bool NUVE::System::isInitialSetup()
{
    return mIsInitialSetup;
}

void NUVE::System::setIsInitialSetup(bool isInitailSetup)
{
    mIsInitialSetup = isInitailSetup;
}

void NUVE::System::ForgetDevice()
{
    mLastInstalledUpdateDate = {};
    mIsManualUpdate = false;

    QSettings settings;
    settings.setValue(m_updateOnStartKey, false);
    settings.setValue(m_InstalledUpdateDateSetting, mLastInstalledUpdateDate);
    settings.setValue(m_IsManualUpdateSetting, mIsManualUpdate);

    mSync->ForgetDevice();
}

bool NUVE::System::updateSequenceOnStart()
{
    QSettings settings;
    auto update = settings.value(m_updateOnStartKey);
    settings.setValue(m_updateOnStartKey, false);

    return update.isValid() && update.toBool();
}

bool NUVE::System::hasForceUpdate()
{
    return mHasForceUpdate;
}

void NUVE::System::setTestMode(bool testMode) {
    if (mTestMode == testMode)
        return;

    mTestMode = testMode;
    emit testModeChanged();
}

void NUVE::System::setPartialUpdateProgress(int progress) {
    mPartialUpdateProgress = progress;
    emit partialUpdateProgressChanged();
}

void NUVE::System::partialUpdate(const bool isBackdoor) {
#ifdef __unix__
    // Check
    QStorageInfo storageInfo (mUpdateDirectory);

    if (!storageInfo.isValid()) {
        mountUpdateDirectory();
    }

    if (!storageInfo.isValid() || !storageInfo.isReady()) {
        emit error("The update directory is not ready.");
        return;
    }
#endif

    checkAndDownloadPartialUpdate(mLatestVersionKey, isBackdoor);
}

void NUVE::System::partialUpdateByVersion(const QString version)
{
    checkAndDownloadPartialUpdate(version, false, true);
}

void NUVE::System::checkAndDownloadPartialUpdate(const QString installingVersion, const bool isBackdoor, const bool isResetVersion)
{
    QString versionAddressInServer;
    int updateFileSize;

    if (isBackdoor) {
        versionAddressInServer = "/manual_update/" + mBackdoorFileName;
        updateFileSize = mBackdoorUpdateFileSize;

    } else {
        auto versionObj = mUpdateJsonObject.value(installingVersion).toObject();
        versionAddressInServer = versionObj.value(m_Address).toString();

        mUpdateFileSize = versionObj.value(m_CurrentFileSize).toInt();
        updateFileSize = mUpdateFileSize;

        mRequiredMemory = versionObj.value(m_RequiredMemory).toInt();
        m_expectedUpdateChecksum = QByteArray::fromHex(versionObj.value(m_CheckSum).toString().toLatin1());
    }


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

        if (verifyDownloadedFiles(downloadedData, false, isBackdoor, isResetVersion))
            return;
        else
            TRACE << "The file update needs to be redownloaded.";
    }

    if (storageInfo.bytesFree() < updateFileSize) {

        QDir dir(mUpdateDirectory);
        // Removes the directory, including all its contents.
        dir.removeRecursively();

// Create the latestVersion directory
#ifdef __unix__
        TRACE << "Device mounted successfully." << QProcess::execute("/bin/bash", {"-c", "mkdir /mnt/update/latestVersion"});
#endif

        if (storageInfo.bytesFree() < updateFileSize) {
            emit error(QString("The update directory has no memory. Required memory is %0, and available memory is %1.")
                           .arg(QString::number(updateFileSize), QString::number(storageInfo.bytesFree())));
            return;
        }
    }

    if (mNetManager->property(m_isBusyDownloader).toBool()) {
        // To open progress bar.
        emit downloadStarted();
        return;
    }

    emit downloadStarted();

    // Fetch the file from web location
    QNetworkReply* reply = mNetManager->get(QNetworkRequest(m_updateServerUrl.resolved(QUrl(versionAddressInServer))));
    reply->setProperty(m_methodProperty, isBackdoor ? m_backdoorUpdate : m_partialUpdate);
    mNetManager->setProperty(m_isBusyDownloader, true);
    mNetManager->setProperty(m_isResetVersion, isResetVersion);

    setPartialUpdateProgress(0);

    if (mElapsedTimer.isValid())
        mElapsedTimer.invalidate();

    mElapsedTimer.restart();

    mDownloadBytesReceived = 0;

    mRemainingDownloadTime = "Connecting...";
    mDownloadRateEMA = 0;
    emit remainingDownloadTimeChanged();

    connect(reply, &QNetworkReply::finished, this, [=]() {
        downloaderTimer.stop();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [=]() {
        downloaderTimer.stop();
    });

    // disconnect any previous session
    downloaderTimer.disconnect();

    // The downloader has been open for more than 30 seconds and has not received any bytes
    connect(&downloaderTimer, &QTimer::timeout, reply, [=]() {
        double secTime = mElapsedTimer.elapsed() / 1000.0;

        if (mElapsedTimer.isValid() && secTime >= 30) {
            reply->abort();
            downloaderTimer.stop();

        } if (mElapsedTimer.isValid() && secTime >= 5) {
            double rate = 0;
            // Adjust smoothing factor (0.1) as needed
            mDownloadRateEMA = (0.1 * rate) + (0.9 * mDownloadRateEMA);

            auto totalBytes = downloaderTimer.property("totalBytes").toInt();
            int remainTime = mDownloadRateEMA < 0.001 ? 1000000 : qRound((totalBytes - mDownloadBytesReceived) / mDownloadRateEMA);

            QString unit = remainTime < 60 ? "second" : "minute";

            remainTime = remainTime < 60 ? remainTime : qRound(remainTime / 60.0);

            if (remainTime > 1)
                unit += "s";

            mRemainingDownloadTime = QString("About %1 %2 remaining").arg(QString::number(remainTime), unit);
            emit remainingDownloadTimeChanged();
        }
    });

    downloaderTimer.start(1000);

    connect(reply, &QNetworkReply::downloadProgress, this, [=] (qint64 bytesReceived, qint64 bytesTotal) {
        double secTime = mElapsedTimer.elapsed() / 1000.0;
        if (secTime < 1.5 || bytesTotal == 0) {
            return;
        }

        mElapsedTimer.restart();

        double rate = (bytesReceived - mDownloadBytesReceived) / secTime;
        mDownloadBytesReceived = bytesReceived;
        // Adjust smoothing factor (0.2) as needed
        mDownloadRateEMA = (0.2 * rate) + (0.8 * mDownloadRateEMA);

        auto remain = bytesTotal - bytesReceived ;
        int remainTime = mDownloadRateEMA < 0.001 ? 1000000 : qRound(remain / mDownloadRateEMA);

        downloaderTimer.setProperty("totalBytes", bytesTotal);

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

void NUVE::System::updateAndRestart(const bool isBackdoor, const bool isResetVersion)
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

    auto updateFileSize = isBackdoor ? mBackdoorUpdateFileSize : mUpdateFileSize;
    if (updateStorageInfo.bytesFree() < updateFileSize) {

        QString err = QString("The update directory has no memory for intallation.\nRequired memory is %0 bytes, and available memory is %1 bytes.")
                          .arg(QString::number(updateFileSize), QString::number(updateStorageInfo.bytesFree()));
        emit error(err);
        TRACE << err;

        return;
    }

    QStorageInfo installStorageInfo (qApp->applicationDirPath());
    QFileInfo appInfo(qApp->applicationFilePath());

    auto requiredMemory = isBackdoor ? mBackdoorRequiredMemory : mRequiredMemory;;
    if ((installStorageInfo.bytesFree() + appInfo.size()) < requiredMemory) {
        QString err = QString("The update directory has no memory for intallation.\nRequired memory is %0 bytes, and available memory is %1 bytes.")
                          .arg(QString::number(requiredMemory), QString::number(installStorageInfo.bytesFree()));
        emit error(err);
        TRACE << err;

        return;
    }

    TRACE << "starting update" ;

#ifdef __unix__
    // It's incorrect if the update process failed,
    // but in that case, the update is available and
    // this property remains hidden.
    mLastInstalledUpdateDate = QDate::currentDate().toString("dd MMM yyyy");
    QSettings setting;
    setting.setValue(m_InstalledUpdateDateSetting, mLastInstalledUpdateDate);

    // No need to update mIsManualUpdate
    mIsManualUpdate = isBackdoor || isResetVersion;
    setting.setValue(m_IsManualUpdateSetting, mIsManualUpdate);

    // Write unsaved data to settings
    setting.sync();

    emit lastInstalledUpdateDateChanged();

    emit systemUpdating();

    installUpdateService();

    QTimer::singleShot(200, this, [=]() {
        int exitCode = QProcess::execute("/bin/bash", {"-c", "systemctl enable appStherm-update.service; systemctl start appStherm-update.service"});
        TRACE << exitCode;
    });
#endif


}


// Checksum verification after download
bool NUVE::System:: verifyDownloadedFiles(QByteArray downloadedData, bool withWrite, bool isBackdoor, const bool isResetVersion) {
    QByteArray downloadedChecksum = calculateChecksum(downloadedData);

    auto expectedBA = isBackdoor ? mExpectedBackdoorChecksum : m_expectedUpdateChecksum;
    if (downloadedChecksum == expectedBA) {

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

        emit partialUpdateReady(isBackdoor, isResetVersion);

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

            auto method  = netReply->property(m_methodProperty).toString();
            if ( method == m_updateFromServer) {
                qWarning() << "Unable to download updateInfo.json file: " << netReply->errorString();
                // emit alert("Unable to download update information, Please check your internet connection: " + netReply->errorString());

            } else if (method == m_partialUpdate || method == m_backdoorUpdate) {
                mNetManager->setProperty(m_isBusyDownloader, false);
                emit error("Download error: " + netReply->errorString());

                if (isInitialSetup() && method == m_partialUpdate) {
                    static int i = 0;
                    i++;
                    if (i > 5) {
                        // After retry 2 times, the update back to normal state.
                        setIsInitialSetup(false);
                        emit updateChecked();

                    } else {
                        // In initial setup, retry when an error occurred.
                        QTimer::singleShot(10000, this, [this]() {
                                getUpdateInformation(true);

                        });
                    }
                }

            } else {
                qWarning() << "Network/request Error: " << netReply->errorString() << method;
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

    } break;
    case QNetworkAccessManager::GetOperation: {

        // Partial update (download process) finished.
        if (netReply->property(m_methodProperty).toString() == m_partialUpdate) {

            // Check data and prepare to set up.
            verifyDownloadedFiles(data, true, false, netReply->property(m_isResetVersion).toBool());
            mNetManager->setProperty(m_isBusyDownloader, false);

        } else if (netReply->property(m_methodProperty).toString() == m_backdoorUpdate) {

            // Check data and prepare to set up.
            verifyDownloadedFiles(data, true, true);
            mNetManager->setProperty(m_isBusyDownloader, false);

        } else if (netReply->property(m_methodProperty).toString() == m_updateFromServer) { // Partial update (download process) finished.

            TRACE << mUpdateFilePath;
            // Save the downloaded data
            if (checkUpdateFile(data)) {

                QFile file(mUpdateFilePath);
                if (!file.open(QIODevice::WriteOnly)) {
                    TRACE << "Unable to open file for writing";
                    emit error("Unable to open file for writing");
                    break;
                }
                TRACE << doc.toJson().toStdString().c_str();

                file.write(data);

                file.close();
            } else {
                TRACE << "The update information did not fetched correctly, Try again later!" << data.toStdString().c_str();
//                emit alert("The update information did not fetched correctly, Try again later!");
            }

            // Check the last saved updateInfo.json file
            checkPartialUpdate(netReply->property(m_notifyUserProperty).toBool());

        }  else if (netReply->property(m_methodProperty).toString() == m_backdoorFromServer) {

            // Save the downloaded data
            QFile file(qApp->applicationDirPath() + "/files_info.json");
            if (!file.open(QIODevice::WriteOnly)) {
                TRACE << "Unable to open file for writing";
                emit error("Unable to open file for writing");
                break;
            }
            TRACE << "Backdoor Data: " << doc.toJson().toStdString().c_str();

            file.write(data);

            file.close();
        }

    } break;

    default:

        break;
    }

    netReply->deleteLater();
}

void NUVE::System::onSnReady()
{
    emit snReady();

    createLogDirectoryOnServer();
    
    //! Get update information when Serial number is ready.
    getUpdateInformation(true);
}

void NUVE::System::createLogDirectoryOnServer()
{
    auto sn = serialNumber();
    // Check serial number
    if (sn.isEmpty())
        return;

    mLogRemoteFolder = m_logPath + sn;
    // Create remote path in case it doesn't exist, needed once! with internet access
    QString createPath = QString("/usr/local/bin/sshpass -p '%1' ssh -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2@%3 'mkdir -p %4'").
                         arg(m_logPassword, m_logUsername, m_logServerAddress, mLogRemoteFolder);
    mLogSender.start("/bin/bash", {"-c", createPath});
}

bool NUVE::System::checkUpdateFile(const QByteArray updateData) {
    auto updateDoc = QJsonDocument::fromJson(updateData);
    if (updateDoc.isNull()) {
        qWarning() << "The update information has invalid format (server side).";
        return false;
    }

    auto updateJson = updateDoc.object();

    // Find the maximum version
    QString latestVersionKey = findLatestVersion(updateJson);

    // Save the file
    if (latestVersionKey.isEmpty())
        return true;

    auto latestVersionObj = updateJson.value(latestVersionKey).toObject();

    if (latestVersionKey.split(".").count() == 3) {

        QStringList jsonKeys;
        jsonKeys << m_ReleaseDate
                 << m_ChangeLog
                 << m_Address
                 << m_RequiredMemory
                 << m_CurrentFileSize
                 << m_CheckSum
                 << m_Staging
                 << m_ForceUpdate;

        if (latestVersionObj.isEmpty()) {
            qWarning() << "The 'LatestVersion' value (" << latestVersionKey << ") is empty in the Update file (server side).";
            return false;
        }

        foreach (auto key, jsonKeys) {
            auto value = latestVersionObj.value(key);
            if (value.isUndefined() || value.type() == QJsonValue::Null) {
                qWarning() << "The key (" << key << ") not found in the 'LatestVersion' value (" << latestVersionKey << ") (server side).";
                return false;
            }

            if (value.isString() && value.toString().isEmpty()) {
                qWarning() << "The key (" << key << ") is empty in the 'LatestVersion' value (" << latestVersionKey << ") (server side).";
                return false;

            } else if (value.isDouble() && (value.toDouble(-100) == -100)) {
                qWarning() << "The key (" << key << ") is empty in the 'LatestVersion' value (" << latestVersionKey << ") (server side).";
                return false;
            }
        }

    } else {
        qWarning() << "The 'LatestVersion' value (" << latestVersionKey << ") is incorrect in the Update file (server side).";
        return false;
    }

    return true;
}

void NUVE::System::setUID(cpuid_t uid)
{
    mUID = uid;
    mSync->setUID(uid);
    emit systemUIDChanged();
}

QString NUVE::System::systemUID()
{
    return QString::fromStdString(mUID);
}

void NUVE::System::checkPartialUpdate(bool notifyUser, bool installLatestVersion) {

    // Read the downloaded data
    QFile file(mUpdateFilePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        TRACE << "Unable to open file for reading";
        return;
    }

    mUpdateJsonObject = QJsonDocument::fromJson(file.readAll()).object();
    mUpdateJsonObject.remove("LatestVersion");

    file.close();

    updateAvailableVersions(mUpdateJsonObject);

    mHasForceUpdate = false;
    auto latestVersionKey = findLatestVersion(mUpdateJsonObject);
    auto installableVersionKey = installLatestVersion ? latestVersionKey : findForceUpdate(mUpdateJsonObject);

    if (installableVersionKey.isEmpty())
        installableVersionKey = latestVersionKey;


    TRACE << "Installable version: " << installableVersionKey << "Maximum Version:" << latestVersionKey;

    if (installableVersionKey.isEmpty())
        return;

    auto latestVersionObj = mUpdateJsonObject.value(installableVersionKey).toObject();

    // Check version (app and latest)
    auto currentVersion = qApp->applicationVersion();



    auto releaseDate = QDate::fromString(latestVersionObj.value(m_ReleaseDate).toString(), "d/M/yyyy");
    auto releaseDateStr = releaseDate.isValid() ? releaseDate.toString("dd MMM yyyy") : latestVersionObj.value(m_ReleaseDate).toString();
    auto changeLog = latestVersionObj.value(m_ChangeLog).toString();


    mLatestVersionChangeLog = "V" + installableVersionKey + ":\n\n" + changeLog;


    if (mLatestVersionKey  != installableVersionKey ||
        mLatestVersionDate != releaseDateStr) {

        mLatestVersionKey  = installableVersionKey;
        mLatestVersionDate = releaseDateStr;

        if (mLastInstalledUpdateDate.isEmpty())
            mLastInstalledUpdateDate = mLatestVersionDate;

        emit latestVersionChanged();
    }

    // Compare versions lexicographically
    // installableVersionKey > currentVersion
    if (isVersionNewer(installableVersionKey, currentVersion)) {
        setUpdateAvailable(true);
        mHasForceUpdate = latestVersionObj.value(m_ForceUpdate).toBool();

        if (!mHasForceUpdate && notifyUser  && !mIsManualUpdate)
            emit notifyNewUpdateAvailable();
    }

    //! to enable checking update normally after first time checked!
    setIsInitialSetup(false);

    // Check all logs
    updateLog(mUpdateJsonObject);
    emit logVersionChanged();

    // Manual update must be exit for force update
    if (installLatestVersion || (mHasForceUpdate && !mIsManualUpdate)) {
        partialUpdate();
    }

    emit updateChecked();
}

void NUVE::System::updateAvailableVersions(const QJsonObject updateJsonObject)
{
    auto versions = updateJsonObject.keys();

    if (mAvailableVersions != versions) {
        mAvailableVersions = versions;
        emit availableVersionsChanged();
    }
}

QString NUVE::System::getLogByVersion(const QString version)
{
    if (mUpdateJsonObject.empty())
        return QString();

    auto obj = mUpdateJsonObject.value(version).toObject();
    return ("V" + version + ":\n\n" + obj.value(m_ChangeLog).toString());
}

void NUVE::System::updateLog(const QJsonObject updateJsonObject)
{
    auto versions = updateJsonObject.keys();
    if (versions.contains("LatestVersion"))
        versions.removeOne("LatestVersion");

    // The current version log added.
    versions.removeOne(mLatestVersionKey);

    std::sort(versions.begin(), versions.end(), isVersionNewer);

    // Check version (app and latest)
    auto currentVersion = qApp->applicationVersion();

    foreach (auto keyVersion, versions) {
        auto compareVersion = isVersionNewer(keyVersion, currentVersion);
        if (compareVersion && isVersionNewer(mLatestVersionKey, keyVersion)) {
            auto obj = updateJsonObject.value(keyVersion).toObject();
            mLatestVersionChangeLog += ("\n\nV" + keyVersion + ":\n\n" + obj.value(m_ChangeLog).toString());
        } else if (!compareVersion) {
            break;
        }
    }
}

QString NUVE::System::findForceUpdate(const QJsonObject updateJsonObject)
{
    auto versions = updateJsonObject.keys();
    if (versions.contains("LatestVersion"))
        versions.removeOne("LatestVersion");


    // Last force update.
    std::sort(versions.begin(), versions.end(), isVersionNewer);

    // Check version (app and latest)
    auto currentVersion = qApp->applicationVersion();

    // return the last force update that is greater than the current version
    foreach (auto keyVersion, versions) {
        if (isVersionNewer(keyVersion, currentVersion)) {
            auto obj = updateJsonObject.value(keyVersion).toObject();
            if (obj.value(m_ForceUpdate).toBool()) {
                return keyVersion;
            }
        }
    }

    return QString();
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

void NUVE::System::stopDevice()
{
#ifdef __unix__
    QTimer::singleShot(500, this, [](){
        int exitCode = QProcess::execute("/bin/bash", {"-c", "systemctl disable appStherm.service;systemctl stop appStherm.service;"});
        TRACE << exitCode;
    });
#endif
}

bool NUVE::System::fetchSettings()
{
    return getUpdate();
}

QString NUVE::System::findLatestVersion(QJsonObject updateJson) {
    QStringList versions = updateJson.keys();
    if (versions.contains("LatestVersion"))
        versions.removeOne("LatestVersion");

    std::sort(versions.begin(), versions.end(), isVersionNewer);

    TRACE << versions;
    // Find the maximum version
    QString latestVersionKey;

    foreach (auto ver, versions) {
        auto latestVersionObj = updateJson.value(ver).toObject();
        if (mTestMode || !latestVersionObj.value(m_Staging).toBool()) {
            latestVersionKey = ver;
            break;
        }
    }

    return latestVersionKey;
}

QStringList NUVE::System::cpuInformation() {

    QStringList cpuTempList;

    for (int i = 0; ; ++i) {
        QString fileName = QString("/sys/class/thermal/thermal_zone%1/temp").arg(i);
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // No more thermal zones found, exit the loop
            break;
        }

        QTextStream in(&file);
        QString line = in.readLine();
        if (!line.isEmpty()) {
            bool ok;
            int temperature = line.toInt(&ok);
            if (ok) {
                cpuTempList.append(line);
                TRACE << "CPU" << i << "Temperature:" << temperature << "mili Centigrade";
            } else {
                cpuTempList.append("invalid");
                TRACE << "Failed to parse temperature for CPU" << i;
            }
        } else {
            TRACE << "Empty temperature file for CPU" << i;
        }

        file.close();
    }

    return cpuTempList;
}

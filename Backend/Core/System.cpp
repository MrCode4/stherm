#include "System.h"
#include "LogHelper.h"

#include <QProcess>
#include <QDebug>
#include <QUrl>

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
const QString m_updateServerUrl  = "http://fileserver.nuvehvac.com"; // New server
const QString m_logUsername = "uploadtemp";
const QString m_logPassword = "oDhjPTDJYkUOvM9";
const QString m_logServerAddress = "logs.nuvehvac.com";
const QString m_logPath = "/opt/logs/";

const QString m_checkInternetConnection = QString("checkInternetConnection");
const QString m_updateService   = QString("/etc/systemd/system/appStherm-update.service");
const QString m_restartAppService   = QString("/etc/systemd/system/appStherm-restart.service");

//! Path of update file in the server
const QString m_updateInfoFile  = QString("updateInfoV1.json");
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
const QString m_IsManualUpdateSetting      = QString("Stherm/IsManualUpdate");
const QString m_IsFWServerUpdateSetting    = QString("Stherm/IsFWServerUpdate");

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

NUVE::System::System(NUVE::Sync *sync, QObject *parent)
    : RestApiExecutor(parent)
    , mSync(sync)
    , mAreSettingsFetched(false)
    , mUpdateAvailable (false)
    , mHasForceUpdate(false)
    , mIsInitialSetup(false)
    , mTestMode(false)
    , mIsNightModeRunning(false)
    , mRestarting(false)
    , sshpassInstallCounter(0)
{
    mUpdateFilePath = qApp->applicationDirPath() + "/" + m_updateInfoFile;

    connect(mSync, &NUVE::Sync::settingsFetched, this, [this](bool success) {
        mAreSettingsFetched = success;
        emit areSettingsFetchedChanged(success);
    });
    connect(mSync, &NUVE::Sync::serialNumberChanged, this, &NUVE::System::serialNumberChanged);
    connect(mSync, &NUVE::Sync::contractorInfoReady, this, &NUVE::System::contractorInfoReady);

    connect(&mUpdateTimer, &QTimer::timeout, this, [=]() {
        if (!mIsNightModeRunning)
            fetchUpdateInformation(true);
    });

    mUpdateTimer.setInterval(6 * 60 * 60 * 1000); // each 6 hours
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

    mStartedWithFWServerUpdate = setting.value(m_IsFWServerUpdateSetting, false).toBool();

    // reformat if it was saved with old format
    auto oldFormatDate = QDate::fromString(mLastInstalledUpdateDate, "dd/MM/yyyy");
    if (oldFormatDate.isValid())
        mLastInstalledUpdateDate = oldFormatDate.toString("dd MMM yyyy");

    connect(mSync, &NUVE::Sync::serialNumberReady, this, &NUVE::System::onSerialNumberReady);
    connect(mSync, &NUVE::Sync::alert, this, &NUVE::System::alert);
    connect(mSync, &NUVE::Sync::settingsReady, this, &NUVE::System::settingsReady);
    connect(mSync, &NUVE::Sync::appDataReady, this, &NUVE::System::appDataReady);

    connect(mSync, &NUVE::Sync::autoModeSettingsReady, this, [this](const QVariantMap& settings, bool isValid) {
        emit autoModeSettingsReady(settings, isValid);
    });
    connect(mSync, &NUVE::Sync::pushFailed, this, &NUVE::System::pushFailed);
    connect(mSync, &NUVE::Sync::testModeStarted, this, &NUVE::System::testModeStarted);
    connect(mSync, &NUVE::Sync::pushSuccess, this, [this]() {
        emit pushSuccess();
    });

    connect(mSync, &NUVE::Sync::autoModePush, this, [this](bool isSuccess) {
        emit autoModePush(isSuccess);
    });

    // Update the device with the version received from the server.
    connect(mSync, &NUVE::Sync::updateFirmwareFromServer, this, [this](QString version) {
        // Downloader is busy ...
        if (downloaderTimer.isActive() || mRestarting) {
            TRACE << "Ignore firmware update (received from server)";
            return;
        }

        if (!version.isEmpty()) {
            // Check with current version
            if (version != qApp->applicationVersion()) {
                TRACE << "Install firmware version from server " << version;
                // We'll try to install the requested version, but if it's unavailable,
                // we keep the current version
                // If a version has been installed manually (manual version),
                // the current version will be retained and server updates will not overwrite it.

                if (!mStartedWithManualUpdate)
                    checkAndDownloadPartialUpdate(version, false, false, true);
            }

        } else if (mStartedWithFWServerUpdate) {
            // Install the latest version, and exit from fw server update
            TRACE << "Install latest version";
            checkPartialUpdate(false, true);
        }
    });

    connect(this, &NUVE::System::systemUpdating, this, [this](){
        QSettings settings;
        settings.setValue(m_updateOnStartKey, true);
    });

    // Check: The downloader has been open for more than 30 seconds and has not received any bytes
    downloaderTimer.setTimerType(Qt::PreciseTimer);
    downloaderTimer.setSingleShot(false);

    connect(&mLogSender, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        qWarning() << "process has encountered an error:" << error << mLogSender.readAllStandardError();

        auto initialized = mLogSender.property("initialized");
        if (initialized.isValid() && initialized.toBool()){
            emit alert("Log is not sent, Please try again!");
        } else {
            createLogDirectoryOnServer();
        }
    });
    connect(&mLogSender, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        bool success = true;
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            qWarning() << "process did not exit cleanly" << exitCode << exitStatus << mLogSender.readAllStandardError()
                       << mLogSender.readAllStandardOutput();
            success = false;
        }

        auto initialized = mLogSender.property("initialized");
        if (!initialized.isValid() || !initialized.toBool()){
            if (success){
                TRACE << "Folder created in server successfully";
                mLogSender.setProperty("initialized", true);
                sendLog();
            } else {
                createLogDirectoryOnServer();
            }
        } else {
            if (success) {
                emit alert("Log is sent!");
            } else {
                emit alert("Log is not sent, Please try again!");
            }
        }
    });

    //! copies the sshpass from /usr/local/bin/ to /usr/bin
    //! if the first one exists and second one not exists
    //! will be installed from server when needed if either not exists
    if (!has_sshPass()) {
        TRACE << "sshpass was not in /usr/bin";
        QFile sshpass_local("/usr/local/bin/sshpass");
        if (sshpass_local.exists()) {
            TRACE << "sshpass copying to /usr/bin";
            auto success = sshpass_local.copy("/usr/bin/sshpass");
            TRACE_CHECK(success) << "copy sshpass successfuly";
            TRACE_CHECK(!success) << "failed to copy sshpass";
        } else {
            TRACE << "sshpass is not in /usr/local/bin either, will be installed on first use";
        }
    }

    if (!serialNumber().isEmpty())
        onSerialNumberReady();
}

NUVE::System::~System()
{
}

bool NUVE::System::areSettingsFetched() const {return mAreSettingsFetched;}

bool NUVE::System::installSystemCtlRestartService()
{
    #ifdef __unix__
    QFile systemctlRestartSH("/usr/local/bin/systemctlRestart.sh");
    if (systemctlRestartSH.exists())
        systemctlRestartSH.remove("/usr/local/bin/systemctlRestart.sh");

    QFile copyFile(":/Stherm/systemctlRestart.sh");
    if (!copyFile.copy("/usr/local/bin/systemctlRestart.sh")) {
        TRACE << "systemctlRestart.sh file did not updated: " << copyFile.errorString();
        return false;
    }

    auto exitCode = QProcess::execute("/bin/bash", {"-c", "chmod +x /usr/local/bin/systemctlRestart.sh"});
    if (exitCode == -1 || exitCode == -2)
        return false;

    QFile updateServiceFile(m_restartAppService);

    QString serviceContent = "[Unit]\n"
                             "Description=Nuve Smart HVAC system restart service\n"
                             "[Service]\n"
                             "Type=simple\n"
                             "ExecStart=/bin/bash -c \"/usr/local/bin/systemctlRestart.sh\"\n"
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

        TRACE << "The systemctlRestart service successfully installed.";

    } else if (!neetToUpdateService) {
        TRACE << "The service is already installed..";

    } else {
        TRACE << "Unable to install the systemctlRestart service.";

        return false;
    }

    // Disable the appStherm-restart
    return updateServiceState("appStherm-restart", false);
    #endif

    return true;
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
    return updateServiceState("appStherm-update", false);

#endif
    return true;
}

bool NUVE::System::installSSHPass(bool recursiveCall)
{
    if (!recursiveCall)
        sshpassInstallCounter = 0;

    TRACE << "Check sshpass existence" << recursiveCall << sshpassInstallCounter;

    if (sshpassInstallCounter > 3)
        return false;

    sshpassInstallCounter++;

#ifdef __unix__
    // this helps validating the existence as well as workable version of sshPass
    auto checkExists = []()->bool {
        QProcess process;
        process.start("/bin/bash", {"-c", "sshpass -V"});
        if (!process.waitForStarted())
            return false;

        if (!process.waitForFinished())
            return false;

        QByteArray result = process.readAll();
        TRACE << process.exitCode() << process.exitStatus() << result;
        return (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && result.startsWith("sshpass"));
    };

    if (!checkExists()) {
        QFile getsshpassRun("/usr/local/bin/getsshpass.sh");
        if (getsshpassRun.exists())
            getsshpassRun.remove("/usr/local/bin/getsshpass.sh");

        QFile copyFile(":/Stherm/getsshpass.sh");
        if (!copyFile.copy("/usr/local/bin/getsshpass.sh")) {
            TRACE << "getsshpass.sh file did not updated: " << copyFile.errorString();
            return false;
        }

        auto exitCode = QProcess::execute("/bin/bash", {"-c", "chmod +x /usr/local/bin/getsshpass.sh"});
        if (exitCode == -1 || exitCode == -2)
            return false;

        TRACE << "getting the sshpass using getsshpass.sh";

        QProcess getsshProcess;
        getsshProcess.start("/bin/bash", {"-c", "/usr/local/bin/getsshpass.sh"});
        if (!getsshProcess.waitForStarted())
            return false;

        if (!getsshProcess.waitForFinished())
            return false;

        QByteArray result = getsshProcess.readAll();

        exitCode = getsshProcess.exitCode();
        TRACE << "getsshpass.sh file exec: " << exitCode << getsshProcess.exitStatus() << result;
        if (exitCode < 0)
            return false;

        return installSSHPass(true);
    }
#endif

    return true;
}

bool NUVE::System::updateServiceState(const QString& serviceName, const bool& run)
{
#ifdef __unix__
    QString command = QString("systemctl disable %0.service;").arg(serviceName);
    if (run) {
        command = QString("systemctl enable %0.service; systemctl start %0.service").arg(serviceName);
    }

    auto exitCode = QProcess::execute("/bin/bash", {"-c", command});
    TRACE << command << "- exitCode: " << exitCode;
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
        fetchBackdoorInformation();

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
    if (mLogSender.state() != QProcess::NotRunning){
        QString error("Previous session is in progress, please try again later.");
        qWarning() << error << "State is :" << mLogSender.state();
        emit alert(error);
        return;
    }

    auto initialized = mLogSender.property("initialized");
    if (!initialized.isValid() || !initialized.toBool()){
        qWarning() << "Folder was not created successfully, trying again...";
        createLogDirectoryOnServer(); // will call the sendLog if successful
        return;
    }

    QString filename = "/mnt/log/log/" + QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmss") + ".log";

    TRACE << "generating log file in " << filename;

    // Create log
    auto exitCode = QProcess::execute("/bin/bash", {"-c", "journalctl -u appStherm > " + filename});
    if (exitCode < 0)
    {
        QString error("Unable to create log file.");
        qWarning() << error << exitCode;
        emit alert(error);
        return;
    }

    // Copy file to remote path, should be execute detached but we should prevent a new one before current one finishes
    QString copyFile = QString("sshpass -p '%1' scp  -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2 %3@%4:%5").
                       arg(m_logPassword, filename, m_logUsername, m_logServerAddress, mLogRemoteFolder);
    TRACE << "sending log to server " << mLogRemoteFolder;
    mLogSender.start("/bin/bash", {"-c", copyFile});
}

void NUVE::System::systemCtlRestartApp()
{
    #ifdef __unix__
    installSystemCtlRestartService();

    QTimer::singleShot(200, this, [=]() {
        updateServiceState("appStherm-restart", true);
    });
    #endif
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

bool NUVE::System::hasClient() const {return mSync->hasClient();}

void NUVE::System::fetchSerialNumber(const QString& uid, bool notifyUser)
{
    mSync->fetchSerialNumber(uid, notifyUser);

    if (mSync->hasClient()) {
        setUID(uid.toStdString());
    }
}

void NUVE::System::fetchUpdateInformation(bool notifyUser)
{
    bool installLatest = mIsManualUpdate ? false : notifyUser;

    auto callback = [this, installLatest](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() != QNetworkReply::NoError) {
            auto err = "Unable to download " + m_updateInfoFile + " file: " + reply->errorString();
            qWarning() << err;
            emit fetchUpdateErrorOccurred(err);
        }
        else {
            TRACE << mUpdateFilePath;
            // Save the downloaded data
            if (checkUpdateFile(rawData)) {
                QFile file(mUpdateFilePath);
                if (!file.open(QIODevice::WriteOnly)) {
                    auto err = QString("Unable to open file for writing");
                    TRACE << err;
                    emit error(err);
                    emit fetchUpdateErrorOccurred(err);
                    return;
                }

                file.write(rawData);
                file.close();
            }
            else {
                QString err = "The update information did not fetched correctly, Try again later!";
                emit fetchUpdateErrorOccurred(err);
                TRACE << err;
            }

            // Check the last saved m_updateInfoFile file
            checkPartialUpdate(installLatest);
        }
    };

    // Fetch the file from web location
    auto reply = downloadFile(m_updateServerUrl + "/" + m_updateInfoFile, callback);
    // skip logging the content
    if (reply) {
        reply->setProperty("noContentLog", true);

    } else {
        emit fetchUpdateErrorOccurred("Skipped fetching information.");
    }
}

QString NUVE::System::fetchUpdateInformationSync(bool notifyUser)
{
    QEventLoop loop;
    QString error;

    fetchUpdateInformation(notifyUser);
    // error
    connect(this, &NUVE::System::fetchUpdateErrorOccurred, &loop, [&error, &loop] (QString err) {
        error = "Unable to fetch update. Please retry.\n" + err;
        loop.quit();
    });
    QTimer::singleShot(30000, &loop, [&error, &loop] {
        error = "Unable to fetch update. Please retry. Timeout!" ;
        loop.quit();
    });

    // force update available
    connect(this, &NUVE::System::forceUpdateChanged, &loop, [this, &error, &loop] () {
        error = mHasForceUpdate ? "Applying mandatory update. Please wait..." : "";
        loop.quit();
    });
    // update available but not force
    connect(this, &NUVE::System::notifyNewUpdateAvailable, &loop, [&error, &loop] () {
        loop.quit();
    });
    // update not available
    connect(this, &NUVE::System::updateNoChecked, &loop, [&loop] () {
        loop.quit();
    });

    loop.exec();

    return error;
}

void NUVE::System::fetchBackdoorInformation()
{
    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            // Save the downloaded data
            QFile file(qApp->applicationDirPath() + "/files_info.json");
            if (file.open(QIODevice::WriteOnly)) {
                TRACE << "Backdoor Data saved.";
                file.write(rawData);
                file.close();
            }
            else {
                TRACE << "Unable to open file for writing";
                emit error("Unable to open file for writing");
            }
        }
    };

    // Fetch the backdoor file from web location
    auto reply = downloadFile(m_updateServerUrl + "/manual_update/files_info.json", callback);
    // skip logging the content
    if (reply) {
        reply->setProperty("noContentLog", true);
    }
}

void NUVE::System::wifiConnected(bool hasInternet) {
    if (!hasInternet) {
        mUpdateTimer.stop();
        return;
    }

    mUpdateTimer.start();
    if (!mIsNightModeRunning) {

        // In initial setup process should not trigger the fetchUpdateInformation function immediately.
        // Instead, we wait until the serial number of the device is available.
        // Once the serial number is ready, the onSerialNumberReady signal is emitted,
        // prompting the call to fetchUpdateInformation.
        // This ensures information is updated only after the device is fully initialized.
        if (!mIsInitialSetup)
            fetchUpdateInformation(true);

        fetchBackdoorInformation();
    }
}

void NUVE::System::pushSettingsToServer(const QVariantMap &settings)
{
    mSync->pushSettingsToServer(settings);
}

void NUVE::System::pushAutoSettingsToServer(const double& auto_temp_low, const double& auto_temp_high)
{
    mSync->pushAutoSettingsToServer(auto_temp_low, auto_temp_high);
}

QString NUVE::System::getCurrentTime()
{
    // Retrieve utc time from the internet if available; otherwise, use the local system time (UTC).
    auto time = QDateTime::currentDateTimeUtc();

    QEventLoop* eventLoop = nullptr;
    auto callback = [this, &eventLoop, &time](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {

        // Convert string to QDateTime to validate the received time.
        auto dateTime = QDateTime::fromString(data.value("utc_datetime").toString(), Qt::ISODate);

        if (dateTime.isValid())
            time = dateTime;

        TRACE << "getCurrentTime: " << data.value("utc_datetime").toString() << dateTime;

        if (eventLoop) {
            eventLoop->quit();
        }
    };

    auto netReply = callGetApi(QString("https://worldtimeapi.org/api/timezone/Etc/UTC"), callback, false);

    if (netReply) {
        netReply->ignoreSslErrors();
        QEventLoop loop;
        eventLoop = &loop;
        loop.exec();
    }

    return time.toString(Qt::ISODate);
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

void NUVE::System::ignoreManualUpdateMode(bool checkUpdate)
{
    mIsManualUpdate = false;
    // we can use this for early update in some case
    if (checkUpdate)
        checkPartialUpdate(true, false);
}

bool NUVE::System::isFWServerUpdate()
{
    return mStartedWithFWServerUpdate;
}

QVariantMap NUVE::System::getContractorInfo() const
{
    return mSync->getContractorInfo();
}

bool NUVE::System::fetchContractorInfo() {
    return mSync->fetchContractorInfo();
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
    return mSync->getSerialNumber();
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

bool NUVE::System::has_sshPass()
{
    QFileInfo sshPass("/usr/bin/sshpass");

    return sshPass.exists();
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

void NUVE::System::forgetDevice()
{
    mLastInstalledUpdateDate = {};
    mIsManualUpdate = false;
    mAreSettingsFetched = false;

    QSettings settings;
    settings.setValue(m_updateOnStartKey, false);
    settings.setValue(m_InstalledUpdateDateSetting, mLastInstalledUpdateDate);
    settings.setValue(m_IsManualUpdateSetting, mIsManualUpdate);

    mSync->forgetDevice();
}

void NUVE::System::setNightModeRunning(const bool running) {
    if (mIsNightModeRunning == running)
        return;

    mIsNightModeRunning = running;

    if (mIsNightModeRunning) {
        cpuInformation();
    }
}

bool NUVE::System::updateSequenceOnStart()
{
    QSettings settings;
    auto update = settings.value(m_updateOnStartKey);
    // TODO remove later
    auto updateOld = settings.value("m_updateOnStartKey");
    settings.setValue(m_updateOnStartKey, false);
    settings.setValue("m_updateOnStartKey", false);

    return (update.isValid() && update.toBool()) || (updateOld.isValid() && updateOld.toBool());
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

void NUVE::System::checkAndDownloadPartialUpdate(const QString installingVersion, const bool isBackdoor,
                                                 const bool isResetVersion, const bool isFWServerVersion)
{
    QString versionAddressInServer;
    int updateFileSize;

    if (isBackdoor) {
        versionAddressInServer = "/manual_update/" + mBackdoorFileName;
        updateFileSize = mBackdoorUpdateFileSize;

    } else {
        if (!mUpdateJsonObject.contains(installingVersion)) {
            TRACE << "Requested version is not available, version: " << installingVersion;
            return;
        }

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

    emit downloadStarted();

    if (mIsBusyDownloader) {
        return;
    }

    mIsBusyDownloader = true;

    auto callback = [this, isBackdoor, isResetVersion, isFWServerVersion]
        (QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        mIsBusyDownloader = false;
        if (reply->error() == QNetworkReply::NoError) {
            // Check data and prepare to set up.
            if (isBackdoor) {
                verifyDownloadedFiles(rawData, true, true);
            }
            else {
                verifyDownloadedFiles(rawData, true, false, isResetVersion, isFWServerVersion);
            }
        }
        else {
            emit error("Download error: " + reply->errorString());

            if (isInitialSetup() && !isBackdoor) {
                static int i = 0;
                i++;
                if (i > 5) {
                    // After retry 2 times, the update back to normal state.
                    setIsInitialSetup(false);
                    emit updateNoChecked();
                }
                else {
                    // In initial setup, retry when an error occurred.
                    QTimer::singleShot(10000, this, [this]() {
                        fetchUpdateInformation(true);
                    });
                }
            }
        }
    };

    // Fetch the file from web location    
    if (!versionAddressInServer.startsWith("/")) versionAddressInServer = "/" + versionAddressInServer;
    QNetworkReply* reply = downloadFile(m_updateServerUrl + versionAddressInServer, callback, false);
    if (!reply) {
        // another call in progress, so ignore
        TRACE << "Downloading file " << (m_updateServerUrl + versionAddressInServer) << " got called more than once";
        return;
    }

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

void NUVE::System::updateAndRestart(const bool isBackdoor, const bool isResetVersion, const bool isFWServerVersion)
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
    setting.setValue(m_IsFWServerUpdateSetting, isFWServerVersion);

    // Write unsaved data to settings
    setting.sync();

    emit lastInstalledUpdateDateChanged();

    mRestarting = true;
    emit systemUpdating();

    installUpdateService();

    QTimer::singleShot(200, this, [=]() {
        TRACE << updateServiceState("appStherm-update", true);
    });
#endif


}


// Checksum verification after download
bool NUVE::System:: verifyDownloadedFiles(QByteArray downloadedData, bool withWrite, bool isBackdoor,
                                         const bool isResetVersion, const bool isFWServerVersion) {
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

        emit partialUpdateReady(isBackdoor, isResetVersion, isFWServerVersion);

        return true;

    } else if (withWrite) {
        // Checksums don't match - downloaded app might be corrupted
        TRACE << "Checksums don't match - downloaded app might be corrupted";

        emit error("Checksums don't match - downloaded app might be corrupted");
    }

    return false;
}

void NUVE::System::onSerialNumberReady()
{
    emit serialNumberReady();
    fetchUpdateInformation(true);
}

void NUVE::System::createLogDirectoryOnServer()
{
    if (!installSSHPass()){
        QString error("Device is not ready to send log!");
        qWarning() << error;
        emit alert(error);
        return;
    }

    auto sn = serialNumber();
    // Check serial number
    if (sn.isEmpty()){
        QString error("Serial number empty! can not send log!");
        qWarning() << error;
        emit alert(error);
        return;
    }

    int tryCount = mLogSender.property("tryCount").toInt();
    if (tryCount < 3) {
        tryCount++;
        mLogSender.setProperty("tryCount", tryCount);
    } else {
        QString error("Can not send log! Try again later.");
        qWarning() << error;
        emit alert(error);
        tryCount = 0; // reset the counter for next time!
        mLogSender.setProperty("tryCount", tryCount);
        return;
    }

    mLogRemoteFolder = m_logPath + sn;
    // Create remote path in case it doesn't exist, needed once! with internet access
    QString createPath = QString("sshpass -p '%1' ssh -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2@%3 'mkdir -p %4'").
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
                 << m_Staging;

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

void NUVE::System::setSerialNumber(const QString &sn)
{
    mSync->setSerialNumber(sn);
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

    // Update mHasForceUpdate in the findForceUpdate function.
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

    bool manualUpdateInstalled = mIsManualUpdate || mStartedWithFWServerUpdate;

    // Compare versions lexicographically
    // installableVersionKey > currentVersion
    if (isVersionNewer(installableVersionKey, currentVersion)) {
        setUpdateAvailable(true);

        if (!mHasForceUpdate && notifyUser  && !manualUpdateInstalled)
            emit notifyNewUpdateAvailable();
    }

    //! to enable checking update normally after first time checked!
    if (!mUpdateAvailable) {
        setIsInitialSetup(false);
        emit updateNoChecked();
    }

    // Check all logs
    updateLog(mUpdateJsonObject);
    emit logVersionChanged();

    // Manual update must be exit for force update
    if (installLatestVersion || (mHasForceUpdate && !manualUpdateInstalled)) {
        partialUpdate();
    } else {
        TRACE << "update not started" << installLatestVersion << mHasForceUpdate << manualUpdateInstalled;
    }
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

    std::sort(versions.begin(), versions.end(), isVersionNewer);

    // Check version (app and latest)
    auto currentVersion = qApp->applicationVersion();
    QString latestVersionKey;

    foreach (auto keyVersion, versions) {
        if (isVersionNewer(keyVersion, currentVersion)) {
            auto obj = updateJsonObject.value(keyVersion).toObject();
            auto isForce =  obj.value(m_ForceUpdate).toBool(false);

            if (isForce) {
                // Update the earlier force update that is greater than the current version
                if (mTestMode || !obj.value(m_Staging).toBool()) {
                    mHasForceUpdate = true;
                    emit forceUpdateChanged();
                    latestVersionKey = keyVersion;
                }
            }
        } else { // to skip checking further versions which all are lower!
            break;
        }
    }

    return latestVersionKey;
}

void NUVE::System::rebootDevice()
{
#ifdef __unix__
    QProcess process;
    QString command = "reboot";

    mRestarting = true;

    process.start(command);
    process.waitForFinished();

    int exitCode = process.exitCode();
    QByteArray result = process.readAllStandardOutput();
    QByteArray error = process.readAllStandardError();

    mRestarting = false;
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
    return mSync->fetchSettings();
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

bool NUVE::System::checkDirectorySpaces(const QString directory, const uint32_t minimumSizeBytes)
{
#ifdef __unix__
    QStorageInfo storageInfo (directory);

    return (storageInfo.isValid() && storageInfo.bytesFree() >= minimumSizeBytes);
#endif

    return true;
}

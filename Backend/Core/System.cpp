#include "System.h"
#include "Config.h"
#include "LogHelper.h"
#include "PerfTestService.h"
#include "DeviceInfo.h"
#include "NetworkInterface.h"
#include "AppUtilities.h"

#include "ProtoDataManager.h"

#include <QProcess>
#include <QDebug>
#include <QUrl>

Q_LOGGING_CATEGORY(SystemLogCat, "SystemLog")
#define SYS_LOG TRACE_CATEGORY(SystemLogCat)

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
const QString m_updateInfoFile  = QString("updateInfoV11.json");
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
const QString m_NetworkRequestRestartSetting = QString("Stherm/NetworkRequestRestart");

const QString m_updateOnStartKey = "updateSequenceOnStart";
const QString m_LimitedModeRemainigTime = "LimitedModeRemainigTime";
const QString m_InitialSetupWithNoWIFI  = "InitialSetupWithNoWIFI";
const QString m_alternativeNoWiFiFlow   = "alternativeNoWiFiFlow";
const QString m_isDeviceForgotten       = "isDeviceForgotten";

const QString Key_LastRebootAt = "LastRebootCommandAt";

const QString Cmd_PushLogs = "push_logs";
const QString Cmd_PerfTest = "perf_test";
const QString Cmd_Reboot = "reboot";
const QString Cmd_PushLiveData = "push_live_data";
const QString Cmd_ForgetDevice = "forget_device";
const QString Cmd_ForgetDeviceResponse = "forgotten";

//! Function to calculate checksum (Md5)
inline QByteArray calculateChecksum(const QByteArray &data) {
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

//! Function to calculate Hex MD5 of QStrings
inline QString toHexMd5(const QString &data) {
    return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Md5).toHex();
}

inline int parseProgress(const QString &in) {
    QRegularExpression regex(R"((\d+)%\s+\d+)");
    QRegularExpressionMatch match = regex.match(in);

    if (match.hasMatch()) {
        QString progress = match.captured(1);

        bool ok;
        auto progressValue = progress.toInt(&ok);

        if(ok)
            return progressValue;
    }

    // Attempt to parse error code
    QRegularExpression errorRegex(R"(\(code (\d+)\))");
    QRegularExpressionMatch errorMatch = errorRegex.match(in);
    if (errorMatch.hasMatch()) {
        bool ok;

        int errorCode = errorMatch.captured(1).toInt(&ok);

        //! rsync got error
        if (errorCode != 0 || !ok)
            return -2;
    }

    return -1;
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
    , mControlAlertEnabled(false)
    , mTestMode(false)
    , mFactoryTestMode(false)
    , mIsNightModeRunning(false)
    , mRestarting(false)
    , sshpassInstallCounter(0)
    , mFirstLogSent(false)
{
    startAutoSendLogTimer(15 * 60 * 1000); // 15 Minutes

    mUpdateFilePath = qApp->applicationDirPath() + "/" + m_updateInfoFile;

    connect(mSync, &NUVE::Sync::settingsFetched, this, [this](bool success) {
        mAreSettingsFetched = success;
        emit areSettingsFetchedChanged(success);
    });
    connect(mSync, &NUVE::Sync::serialNumberChanged, this, &NUVE::System::serialNumberChanged);

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

    if (!mountDirectory("/mnt/log", "/mnt/log/networkLogs"))
        qWarning() << "unable to create networkLogs folder";

    if (!mountDirectory("/mnt/log", PROTOBUFF_FILES_PATH))
        qWarning() << "unable to create proto folder";

    QSettings setting;
    mLastInstalledUpdateDate = setting.value(m_InstalledUpdateDateSetting).toString();
    mIsManualUpdate          = setting.value(m_IsManualUpdateSetting, false).toBool();
    mStartedWithManualUpdate = mIsManualUpdate;

    // Check if last boot was from command; if so, update the map so that
    // the device does not fall into reboot loop due to same command
    if (setting.contains(Key_LastRebootAt)) {
        mLastReceivedCommands[Cmd_Reboot] = setting.value(Key_LastRebootAt).toString();
    }

    mStartedWithFWServerUpdate = setting.value(m_IsFWServerUpdateSetting, false).toBool();

    // reformat if it was saved with old format
    auto oldFormatDate = QDate::fromString(mLastInstalledUpdateDate, "dd/MM/yyyy");
    if (oldFormatDate.isValid())
        mLastInstalledUpdateDate = oldFormatDate.toString("dd MMM yyyy");

    connect(mSync, &NUVE::Sync::serialNumberReady, this, &NUVE::System::onSerialNumberReady);
    connect(mSync, &NUVE::Sync::alert, this, &NUVE::System::alert);
    connect(mSync, &NUVE::Sync::settingsReady, this, &NUVE::System::settingsReady);
    connect(mSync, &NUVE::Sync::appDataReady, this, &NUVE::System::onAppDataReady);
    connect(mSync, &NUVE::Sync::serviceTitanInformationReady, this, &NUVE::System::serviceTitanInformationReady);

    connect(mSync, &NUVE::Sync::autoModeSettingsReady, this, [this](const QVariantMap& settings, bool isValid) {
        emit autoModeSettingsReady(settings, isValid);
    });
    connect(mSync, &NUVE::Sync::pushFailed, this, &NUVE::System::pushFailed);
    connect(mSync, &NUVE::Sync::testModeStarted, this, [this] () {
        //! Start the factory test mode due to serial number issue.
        setFactoryTestMode(true);
        emit testModeStarted();
    });

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
            auto currentVersion = qApp->applicationVersion();
            currentVersion = AppUtilities::userVersion(currentVersion);

            // Check with current version
            if (version != currentVersion) {
                TRACE << "Install firmware version from server " << version <<
                    "started with manual update" << mStartedWithManualUpdate <<
                    "started with server update" << mStartedWithFWServerUpdate;
                // We'll try to install the requested version, but if it's unavailable,
                // we keep the current version
                // If a version has been installed manually (manual version),
                // the current version will be retained and server updates will not overwrite it.

                if (!mStartedWithManualUpdate)
                    checkAndDownloadPartialUpdate(version, false, false, true);
            }

        } else if (mStartedWithFWServerUpdate) {
            // Install the latest force version back to normal update, and exit from fw server update
            TRACE << "Install latest version if forced";
            mStartedWithFWServerUpdate = false;
            QSettings settings;
            settings.setValue(m_IsFWServerUpdateSetting, false);
            checkPartialUpdate(false, false);
        }
    });

    connect(this, &NUVE::System::systemUpdating, this, [this](){
        QSettings settings;
        settings.setValue(m_updateOnStartKey, true);
    });

    // Check: The downloader has been open for more than 30 seconds and has not received any bytes
    downloaderTimer.setTimerType(Qt::PreciseTimer);
    downloaderTimer.setSingleShot(false);

    //! inits the connections of log sender responses
    mLogSender.initialize([this](QString error){emit logAlert(error);}, "Log sender:", "\r\n");
    //! inits the connections of file sender responses
    mFileSender.initialize([this](QString error){emit testPublishFinished(error);}, "Result sender:");

    //! copies the sshpass from /usr/local/bin/ to /usr/bin
    //! if the first one exists and second one not exists
    //! will be installed from server when needed if either not exists
    if (!has_sshPass()) {
        TRACE << "sshpass was not in /usr/bin or is invalid, so should be remove: "
              << QFile::remove("/usr/bin/sshpass");

        QFile sshpass_local("/usr/local/bin/sshpass");
        bool isValidSshpass_local = sshpass_local.exists() && (sshpass_local.size() > 0);
        if (isValidSshpass_local) {

            TRACE << "sshpass copying to /usr/bin";
            // Invalid file in the /usr/bin/sshpass removed, so copy operation will be successful.
            auto success = sshpass_local.copy("/usr/bin/sshpass");
            TRACE_CHECK(success) << "copy sshpass successfuly";
            TRACE_CHECK(!success) << "failed to copy sshpass";
        } else {
            // Remove the sshpass file when is invalid and exists.
            TRACE << "Remove the invalid sshpass file in /usr/local/bin if exists: " << sshpass_local.remove();

            TRACE << "sshpass is not in /usr/local/bin either, will be installed on first use";
        }
    }

    // Retry with timer to avoid many attempt error.
    mRetryUpdateTimer.setInterval(5000);
    mRetryUpdateTimer.setSingleShot(true);
    connect(&mRetryUpdateTimer, &QTimer::timeout, this, [=]() {
        fetchUpdateInformation(true);
    });

    connect(this, &System::fetchUpdateErrorOccurred, this, [=](QString err) {
        TRACE << "Retry to get update information due to " << err;
        if (isInitialSetup()) {
            mRetryUpdateTimer.start();
        }
    });

    if (!serialNumber().isEmpty())
        onSerialNumberReady();

    connect(&mLogSender, &QProcess::readyReadStandardOutput, this, [&]() {
        if (isBusylogSender()) {
            int progress = parseProgress(mLogSender.readAllStandardOutput());

            // Send only valid values. readAllStandardOutput returns extra non-error outputs like `sending incremental file list\n` and file name.
            if (progress > -1)
                emit sendLogProgressChanged(progress);
            else if (progress == -2)
                emit alert("Sending log failed");
        }
    });

    mStorageMonitor = new StorageMonitor(this);
    mLastLowStorageElapsed.start();
    mLastLowSpaceElapsed.start();
    connect(mStorageMonitor, &StorageMonitor::lowStorageDetected, this, [this]() {
        static int countStorage = 0; // to detect high usage
        auto lastElapsed = mLastLowStorageElapsed.restart();
        SYS_LOG << "low storage detected:" << lastElapsed;

        AppUtilities::removeContentDirectory(mUpdateDirectory);
        SYS_LOG << "update directory removed to make room.";
        if (lastElapsed < 60000) {
            countStorage++;
            SYS_LOG << "log directory removed to make room." << countStorage;
            AppUtilities::removeContentDirectory("/mnt/log/log/");

            if (countStorage > 2) {
                SYS_LOG << "network log directory removed to make room.";
                AppUtilities::removeContentDirectory("/mnt/log/networkLogs/");
            }

            if (countStorage > 5) {
                bool ok = true;
                for (const QString &dirPath : mUsedDirectories) {
                    ok &= AppUtilities::removeContentDirectory(dirPath);
                }
                SYS_LOG << " log partition removed to make room." << ok;
            }

            if (countStorage > 10) {
                SYS_LOG << "Issue with storage detected.";
                // emit error("Your device storage is low.");
                // removeLogPartition() can be called automatically, but it is not advised!
            }
        } else {
            countStorage = 0;
        }
    });

    connect(mStorageMonitor, &StorageMonitor::lowSpaceDetected, this, [this]() {
        static int countSpace = 0; // to detect high usage
        auto lastElapsed = mLastLowSpaceElapsed.restart();
        SYS_LOG << "low space detected:" << lastElapsed;

        SYS_LOG << "old files removed to make some room.";
        QFile::remove("/test_results.csv");
        QFile::remove("/usr/local/bin/updateInfo.json");

        if (lastElapsed < 60000) {
            countSpace++;

            SYS_LOG << "update cache files removed to make some room." << countSpace;
            QFile::remove("/usr/local/bin/files_info.json");

            if (countSpace > 2) {
                SYS_LOG << "custom files remove.";
                QFile::remove("/usr/local/bin/override.ini");
            }

            if (countSpace > 5) {
                SYS_LOG << "Live data removed to make room.";
                QFile::remove("/usr/local/bin/output.bin");
            }

            if (countSpace > 10) {
                SYS_LOG << "Issue with space detected.";
                // emit error("Your device space is low.");
                // or trigger hard factory reset! not advised!
            }
        } else {
            countSpace = 0;
        }
    });
}

NUVE::System::~System()
{
    stopAutoSendLogTimer();
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
        // Remove the sshpass file if is exists or is invalid.
        TRACE << "Remove the /usr/bin/sshpass: "
              << QFile::remove("/usr/bin/sshpass");

        TRACE << "Remove usr/local/bin/sshpass: "
              << QFile::remove("/usr/local/bin/sshpass");

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
        SYS_LOG << "nrf fw mounted to /mnt/update/nrf_fw";
        return true;
    }
    SYS_LOG << "nrf fw did not mount";
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

void NUVE::System::systemCtlRestartApp()
{
    #ifdef __unix__
    setRestartFlag();
    installSystemCtlRestartService();

    QTimer::singleShot(200, this, [=]() {
        updateServiceState("appStherm-restart", true);
    });
    #endif
}

bool NUVE::System::mountDirectory(const QString targetDirectory, const QString targetFolder)
{
    // add to remove on factory reset and to be used in storage manager
    if (!mUsedDirectories.contains(targetFolder))
        mUsedDirectories.append(targetFolder);

#ifdef __unix__
    int exitCode = QProcess::execute("/bin/bash", {"-c", "mkdir "+ targetDirectory + "; mount /dev/mmcblk1p3 " + targetDirectory });
    SYS_LOG << targetDirectory << exitCode;
    if (exitCode < 0)
        return false;

    SYS_LOG << "Device mounted successfully.";

    exitCode = QProcess::execute("/bin/bash", {"-c", "mkdir " + targetFolder});
    SYS_LOG << targetFolder << exitCode;
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
            auto err = "Unable to download update data file: " + reply->errorString();
            qWarning() << "Unable to download " + m_updateInfoFile + " file: " + reply->errorString();
            emit fetchUpdateErrorOccurred(err);
        }
        else {
            TRACE << mUpdateFilePath;
            // Save the downloaded data
            if (checkUpdateFile(rawData)) {
                QFile file(mUpdateFilePath);
                if (!file.open(QIODevice::WriteOnly)) {
                    auto err = QString("Unable to open update file system for writing");
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

    // To validate the network restart requests.
    QSettings settings;
    settings.remove(m_NetworkRequestRestartSetting);

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

void NUVE::System::fetchServiceTitanInformation()
{
    mSync->fetchServiceTitanInformation();
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

void NUVE::System::setLimitedModeRemainigTime(const int &limitedModeRemainigTime) {
    QSettings settings;
    settings.setValue(m_LimitedModeRemainigTime, limitedModeRemainigTime);
}

int NUVE::System::limitedModeRemainigTime() {
    QSettings settings;
    int maxLimit = 100 * 60 * 60 * 1000;
    auto limitedModeRemainigTimeTemp = settings.value(m_LimitedModeRemainigTime, maxLimit).toInt();

    if (limitedModeRemainigTimeTemp > maxLimit)
        limitedModeRemainigTimeTemp = maxLimit;

    return limitedModeRemainigTimeTemp;
}

void NUVE::System::setInitialSetupWithNoWIFI(const bool &initialSetupNoWIFI)
{
    QSettings settings;
    settings.setValue(m_InitialSetupWithNoWIFI, initialSetupNoWIFI);
}

bool NUVE::System::initialSetupWithNoWIFI()
{
    QSettings settings;
    return settings.value(m_InitialSetupWithNoWIFI, false).toBool();
}

bool NUVE::System::isForgottenDeviceStarted()
{
    QSettings settings;
    auto isDeviceForgotten = settings.value(m_isDeviceForgotten, true).toBool();

    SYS_LOG << "Device starts after forgotten: " << isDeviceForgotten;

    return isDeviceForgotten;
}

bool NUVE::System::getRestartFlag()
{
    QSettings settings;
    return settings.value("RestartFlag", false).toBool();
}

void NUVE::System::setRestartFlag()
{
    QSettings settings;
    settings.setValue("RestartFlag", true);
    emit systemAboutToBeShutDown();
}

void NUVE::System::removeRestartFlag()
{
    QSettings settings;
    settings.remove("RestartFlag");
}

void NUVE::System::setIsForgottenDevice(const bool &isDeviceForgotten)
{
    QSettings settings;
    settings.setValue(m_isDeviceForgotten, isDeviceForgotten);
}

void NUVE::System::setAlternativeNoWiFiFlow(const bool &alternativeNoWiFiFlow)
{
    QSettings settings;
    settings.setValue(m_alternativeNoWiFiFlow, alternativeNoWiFiFlow);
}

bool NUVE::System::alternativeNoWiFiFlow()
{
    QSettings settings;
    return settings.value(m_alternativeNoWiFiFlow, false).toBool();
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

    TRACE << "/usr/bin/sshpass information: " << sshPass.exists() << " - size (byte(s)): "  << sshPass.size();

    return sshPass.exists() && (sshPass.size() > 0);
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

bool NUVE::System::controlAlertEnabled()
{
    return mControlAlertEnabled;
}

void NUVE::System::forgetDevice()
{
    mLastInstalledUpdateDate = {};
    mIsManualUpdate = false;
    mAreSettingsFetched = false;

    QSettings settings;
    settings.remove(m_updateOnStartKey);
    settings.remove(m_InstalledUpdateDateSetting);
    settings.remove(m_IsManualUpdateSetting);
    settings.remove(m_InitialSetupWithNoWIFI);

    // User can forget the m_LimitedModeRemainigTime only when the has client is true.
    if (Device->hasClient() || settings.value(m_alternativeNoWiFiFlow, false).toBool())
        settings.remove(m_LimitedModeRemainigTime);

    settings.remove(m_alternativeNoWiFiFlow);

    settings.remove(m_isDeviceForgotten);

    QFile::remove(qApp->applicationDirPath() + "/files_info.json");
    QFile::remove(mUpdateFilePath);
    removeLogPartition();

    mSync->forgetDevice();
}

bool NUVE::System::removeLogPartition(bool removeDirs)
{
    bool ok = true;
    if (removeDirs){
        ok &= AppUtilities::removeDirectory("/mnt/log");
    } else {
        ok &= AppUtilities::removeContentDirectory("/mnt/log");
    }

    return ok;
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
    settings.setValue(m_updateOnStartKey, false);

    return (update.isValid() && update.toBool());
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

void NUVE::System::setControlAlertEnabled(bool enabled)
{
    if (mControlAlertEnabled == enabled)
        return;

    mControlAlertEnabled = enabled;
    emit controlAlertEnabledChanged();
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

    SYS_LOG << mLatestVersionKey << isBackdoor;
    checkAndDownloadPartialUpdate(mLatestVersionKey, isBackdoor);
}

void NUVE::System::partialUpdateByVersion(const QString version)
{
    SYS_LOG << version;
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

    // Checking validation
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
            SYS_LOG << "The file update needs to be redownloaded.";
    }

    // here we just check for the storage needed for app to be downloaded
    // once download is completed and verified, it will be checked for extraction in updateAndRestart
    auto hasFreeBytesDownloading = [&]() {
        SYS_LOG << "byte available" << storageInfo.bytesAvailable() << "update file size" << updateFileSize << "required memory" << mRequiredMemory;

        return (storageInfo.bytesAvailable() > updateFileSize);
    };

    if (!hasFreeBytesDownloading()) {
        // Removes the update directory, including all its contents to make some room.
        AppUtilities::removeContentDirectory(mUpdateDirectory);
        SYS_LOG << "update directory removed to make room for update download.";

        if (!hasFreeBytesDownloading()) {
            // remove more data to make some room for update
            AppUtilities::removeContentDirectory("/mnt/log/log/");
            AppUtilities::removeContentDirectory("/mnt/log/networkLogs/");
            SYS_LOG << "logs directory removed to make room for update download.";

            // show error and return if still has no room
            if (!hasFreeBytesDownloading()) {
                emit error(QString("The update directory has no memory. Required memory is %0, and available memory is %1.")
                               .arg(QString::number(updateFileSize), QString::number(storageInfo.bytesAvailable())));
                return;
            }
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

        SYS_LOG << "Downloading update finished. isBackdoor, isResetVersion, isFWServerVersion, error:" <<
            isBackdoor << isResetVersion << isFWServerVersion << reply->error();

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
            SYS_LOG << "Downloading update error: " << reply->errorString() << isInitialSetup();
            emit error("Download error: " + reply->errorString());

            if (isInitialSetup() && !isBackdoor) {
                static int i = 0;
                i++;
                if (i > 5) {
                    // After retry 5 times, the update back to normal state.
                    setIsInitialSetup(false);
                    emit updateNoChecked();
                }
                else {
                    // In initial setup, retry when an error occurred.
                    mRetryUpdateTimer.start();
                }
            }
        }
    };

    // Fetch the file from web location
    if (!versionAddressInServer.startsWith("/")) versionAddressInServer = "/" + versionAddressInServer;

    QString versionUrlInServer = QString("%0%1?id=%2")
                              .arg(m_updateServerUrl,
                                  versionAddressInServer,
                                  toHexMd5(QString::fromStdString(DeviceInfo::me()->uid())));
    QNetworkReply* reply = downloadFile(versionUrlInServer, callback, false);
    if (!reply) {
        // another call in progress, so ignore
        SYS_LOG << "Downloading file " << (m_updateServerUrl + versionAddressInServer) << " got called more than once";
        return;
    }

    SYS_LOG << "downloading update started" << m_updateServerUrl + versionAddressInServer;
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

    auto requiredMemory = isBackdoor ? mBackdoorRequiredMemory : mRequiredMemory;
    auto updateFileSize = isBackdoor ? mBackdoorUpdateFileSize : mUpdateFileSize;
    // we are checking spece needed here for extraction
    auto hasFreeBytesExtraction = [&]() {
        SYS_LOG << updateStorageInfo.bytesAvailable() <<  updateFileSize << requiredMemory;
        return (updateStorageInfo.bytesAvailable() > requiredMemory);
    };

    if (!hasFreeBytesExtraction()) {
        // remove some logs to make some room for extraction
        AppUtilities::removeContentDirectory("/mnt/log/log/");
        AppUtilities::removeContentDirectory("/mnt/log/networkLogs/");
        SYS_LOG << "logs directory removed to make room for update extraction.";

        if (!hasFreeBytesExtraction()) {
            QString err = QString("The update directory has no memory for intallation.\nRequired memory is %0 bytes, and available memory is %1 bytes.")
                              .arg(QString::number(requiredMemory), QString::number(updateStorageInfo.bytesAvailable()));
            emit error(err);
            SYS_LOG << err;

            return;
        }
    }

    QStorageInfo installStorageInfo (qApp->applicationDirPath());
    QFileInfo appInfo(qApp->applicationFilePath());

    // we check space needed for deployment
    // in this partition byteFree allows us to do copy operation
    auto hasFreeBytesInstallation = [&]() {
        SYS_LOG << installStorageInfo.bytesFree() << installStorageInfo.bytesAvailable() << appInfo.size() <<  requiredMemory;
        return ((installStorageInfo.bytesFree() + appInfo.size()) > requiredMemory);
    };

    if (!hasFreeBytesInstallation()) {

        QFile::remove("/test_results.csv");
        QFile::remove("/usr/local/bin/files_info.json");
        QFile::remove("/usr/local/bin/updateInfo.json");
        SYS_LOG << "Temporary files removed to make room for update installation.";

        if (!hasFreeBytesInstallation()) {
            QString err = QString("The update directory has no memory for intallation.\nRequired memory is %0 bytes, and Free memory is %1 bytes.")
                              .arg(QString::number(requiredMemory), QString::number(installStorageInfo.bytesFree()));
            emit error(err);
            SYS_LOG << err;

            return;
        }
    }

    SYS_LOG << "starting update" ;

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

    setRestartFlag();
    mRestarting = true;
    emit systemUpdating();

    installUpdateService();

    QTimer::singleShot(200, this, [=]() {
        SYS_LOG << updateServiceState("appStherm-update", true);
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
                SYS_LOG << "can not open, update directory path:" << mUpdateDirectory << file.errorString();
                emit error("Unable to open file for writing in " + mUpdateDirectory);
                return false;
            }

            auto wroteSize = file.write(downloadedData);
            file.close();
            if (wroteSize == -1) {
                SYS_LOG << "can not write, update directory path:" << mUpdateDirectory << file.errorString();
                emit error("Unable to write in " + mUpdateDirectory);

                AppUtilities::removeContentDirectory("/mnt/log/log/");
                AppUtilities::removeContentDirectory("/mnt/log/networkLogs/");

                return false;

            } else if (wroteSize != downloadedData.size()) {
                SYS_LOG << "Write corrupted, update directory path:" << mUpdateDirectory << file.errorString();
                emit error("Write corrupted in " + mUpdateDirectory);

                AppUtilities::removeContentDirectory("/mnt/log/log/");
                AppUtilities::removeContentDirectory("/mnt/log/networkLogs/");
                return false;
            }

            // Check update file for validation
            QFile fileRead(mUpdateDirectory + "/update.zip");
            if (fileRead.exists() && fileRead.open(QIODevice::ReadOnly)) {

                auto downloadedData = fileRead.readAll();
                fileRead.close();

                auto verified = verifyDownloadedFiles(downloadedData, false, isBackdoor, isResetVersion, isFWServerVersion);
                if (!verified){
                    SYS_LOG << "Write corrupted after reading, update directory path:" << mUpdateDirectory << fileRead.errorString();
                    emit error("Write corrupted after reading " + mUpdateDirectory);
                }
                return verified;
            }

            SYS_LOG << "Write lost after reading, update directory path:" << mUpdateDirectory << fileRead.errorString();
            emit error("Write lost in " + mUpdateDirectory);
            return false;
        }

        emit partialUpdateReady(isBackdoor, isResetVersion, isFWServerVersion);

        return true;

    } else if (withWrite) {
        // Checksums don't match - downloaded app might be corrupted, we show the error to user only on downloading option.
        emit error("Checksums don't match - downloaded app might be corrupted");
    }

    SYS_LOG << "Checksums don't match - downloaded app might be corrupted" << withWrite << downloadedChecksum << expectedBA;

    return false;
}

void NUVE::System::onSerialNumberReady()
{
    emit serialNumberReady();
}

void NUVE::System::onAppDataReady(QVariantMap data)
{
    emit appDataReady(data);

    if (!data.contains("setting")) return;
    auto command = data.value("setting").toJsonObject().value("command").toString();
    if (command.isEmpty()) return;
    auto commandTime = data.value("setting").toJsonObject().value("command_time").toString();
    attemptToRunCommand(command, commandTime);
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

    // if device started from reboot, let's inform server that the device is booted
    if (mLastReceivedCommands.contains(Cmd_Reboot)) {
        auto callback = [this] (bool success, const QJsonObject& data) {
            mLastReceivedCommands.remove(Cmd_PushLogs); // why not removing Cmd_Reboot
            QSettings settings;
            settings.remove(Key_LastRebootAt);
            SYS_LOG <<"Reporting reboot success. Command cleared" << Cmd_Reboot;
        };
        mSync->reportCommandResponse(callback, Cmd_Reboot, "booted");
    }
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
        SYS_LOG << installLatestVersion << mHasForceUpdate;
        partialUpdate();
    } else {
        SYS_LOG << "update not started" << installLatestVersion << mHasForceUpdate << manualUpdateInstalled;
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
                if ((mTestMode && !mFactoryTestMode) || !obj.value(m_Staging).toBool()) {
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

void NUVE::System::rebootDevice(const bool &isResetFactory)
{
    // In order to avoid writing to settings that have already been cleared in the reset to factory process.
    if (!isResetFactory)
        setRestartFlag();

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
    emit systemAboutToBeShutDown();
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
        if ((mTestMode && !mFactoryTestMode) || !latestVersionObj.value(m_Staging).toBool()) {
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
#ifdef DEBUG_MODE
                TRACE << "CPU" << i << "Temperature:" << temperature << "mili Centigrade";
#endif
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

double NUVE::System::cpuTemperature()
{
    auto cpuInfo = cpuInformation();
    double cpuTemperature = 0;

    if (cpuInfo.length() > 0) {
        bool isOK = false;
        cpuTemperature = cpuInfo.first().toDouble(&isOK);
        if (!isOK) {
            cpuTemperature = 0;
        }
    }

    // Convert mili Centigrade to Centigrade.
    return cpuTemperature / 1000;
}

bool NUVE::System::checkDirectorySpaces(const QString directory, const uint32_t minimumSizeBytes)
{
#ifdef __unix__
    QStorageInfo storageInfo (directory);

    return (storageInfo.isValid() && storageInfo.bytesAvailable() >= minimumSizeBytes);
#endif

    return true;
}

bool NUVE::System::isBusylogSender() const {
    return mLogSender.busy();
}

bool NUVE::System::sendLog(bool showAlert)
{
    if (mLogSender.busy()){
        QString error("Previous session is in progress.");
        SYS_LOG << error << "State is :" << mLogSender.state() << mLogSender.keys();
        return false;
    }

    if (!checkSendLog(showAlert))
        return false;

    auto initialized = mLogSender.property("initialized");
    if (initialized.isValid() && initialized.toBool()) {
        return sendLogFile(showAlert);

    } else {
        qWarning() << "Folder was not created successfully, trying again...";
        auto dirCreatorCallback = [=](QString error) {
            auto role = mLogSender.property("role").toString();
            TRACE_CHECK(role != "dirLog") << "role seems invalid" << role;

            if (!error.isEmpty()) {
                error = "error while creating log directory on remote: " + error;
                qWarning() << error;
                if (showAlert) emit logAlert(error);
                return;
            }

            TRACE << "Folder created in server successfully";
            mLogSender.setProperty("initialized", true);

            sendLogFile(showAlert);
        };

        mLogSender.setRole("dirLog", dirCreatorCallback);

        prepareLogDirectory(dirCreatorCallback);
    }

    return true;
}

void NUVE::System::sendFirstRunLog(bool showAlert)
{
    if (!installSSHPass()) {
        QString error("Device is not ready to send log on First run!");
        qWarning() << error;
        if (showAlert) emit logAlert(error);
        return;
    }

    if (isBusylogSender()){
        QString error("Sending log on first run is in progress!");
        qWarning() << error << "State is :" << mLogSender.state() << mLogSender.keys();
        if (showAlert) emit logAlert(error);
        return;
    }

    auto initialized = mLogSender.property("initializedUID");
    if (initialized.isValid() && initialized.toBool()){
        sendFirstRunLogFile(showAlert);

    } else {
        auto dirCreatorCallback = [=](QString error) {
            auto role = mLogSender.property("role").toString();
            TRACE_CHECK(role != "dirFirstRun") << "role seems invalid" << role;

            if (!error.isEmpty()) {
                error = "error while creating first run log directory on remote: " + error;
                qWarning() << error;
                if (showAlert) emit logAlert(error);
                return;
            }

            mLogSender.setProperty("initializedUID", true);

            sendFirstRunLogFile(showAlert);
        };

        mLogSender.setRole("dirFirstRun", dirCreatorCallback);

        prepareFirstRunLogDirectory();
    }
}

bool NUVE::System::sendResults(const QString &filepath,
                               const QString &remoteIP,
                               const QString &remoteUser,
                               const QString &remotePassword,
                               const QString &destination,
                               bool createDirectory)
{
    if (!installSSHPass()) {
        QString error("Device is not ready to send file!");
        qWarning() << error;
        emit testPublishFinished(error);
        return false;
    }

    if (mFileSender.busy()) {
        QString error("Previous send file session is in progress, please try again later.");
        qWarning() << error << "State is :" << mFileSender.state() << mFileSender.keys();
        emit testPublishFinished(error);
        return false;
    }

    if (!createDirectory){
        sendResultsFile(filepath, remoteIP, remoteUser, remotePassword, destination);

    } else {
        auto dirCreatorCallback = [=](QString error) {
            auto role = mFileSender.property("role").toString();
            TRACE_CHECK(role != "dirFile") << "role seems invalid" << role;

            if (!error.isEmpty()) {
                error = "error while creating directory on remote: " + error;
                qWarning() << error;
                emit testPublishFinished(error);
                return;
            }

            sendResultsFile(filepath, remoteIP, remoteUser, remotePassword, destination);
        };

        mFileSender.setRole("dirFile", dirCreatorCallback);

        prepareResultsDirectory(remoteIP, remoteUser, remotePassword, destination);
    }

    return true;
}

void NUVE::System::prepareLogDirectory(fileSenderCallback callback)
{
    auto sn = serialNumber();
    // Check serial number
    if (sn.isEmpty()){
        QString error("Serial number empty! can not send log!");
        qWarning() << error;
        if (callback) callback(error);
        return;
    }

    TRACE << "creating dir to send log on remote";

    mLogRemoteFolder = m_logPath + sn;
    // Create remote path in case it doesn't exist(-p), needed once! with internet access
    QString createPath = QString("sshpass -p '%1' ssh -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2@%3 'mkdir -p %4'").
                         arg(m_logPassword, m_logUsername, m_logServerAddress, mLogRemoteFolder);
    mLogSender.start("/bin/bash", {"-c", createPath});

}

void NUVE::System::prepareFirstRunLogDirectory()
{
    TRACE << "creating dir to send first log on remote";

    // Create remote path in case it doesn't exist(-p), needed once! with internet access
    mLogRemoteFolderUID = m_logPath + "firstRunLogs";
    QString createPath = QString("sshpass -p '%1' ssh -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2@%3 'mkdir -p %4'").
                         arg(m_logPassword, m_logUsername, m_logServerAddress, mLogRemoteFolderUID);
    mLogSender.start("/bin/bash", {"-c", createPath});
}

void NUVE::System::prepareResultsDirectory(const QString &remoteIP,
                                           const QString &remoteUser,
                                           const QString &remotePassword,
                                           const QString &destination)
{
    TRACE << "creating dir to send results on remote" << destination;

    // it will throw error if folder already exist
    // if server is windows we should add if not exist "dirPath"
    // if server is Linux we should add -p option for mkdir
    // Create remote path in case it doesn't exist, needed once! with internet access
    QString createPath = QString("sshpass -p '%1' ssh -o \"UserKnownHostsFile=/dev/null\" -o "
                                 "\"StrictHostKeyChecking=no\" %2@%3 'mkdir \"%4\"'")
                             .arg(remotePassword, remoteUser, remoteIP, destination);
    mFileSender.start("/bin/bash", {"-c", createPath});
}

QString NUVE::System::generateLog(bool showAlert)
{
    // Validating the storage
    QStorageInfo storageInfo("/mnt/log/log/");

    if (!storageInfo.isValid()) {
        mountDirectory("/mnt/log", "/mnt/log/log");
    }

    if (!storageInfo.isValid() || !storageInfo.isReady()) {
        if (showAlert) emit logAlert("The Logging directory is not ready.");
        return "";
    }

    auto hasFreeBytes = [&]() {
        auto availableBytes = storageInfo.bytesAvailable() / 1024 / 1024;
        SYS_LOG << "byte available in MB" << availableBytes;

        return (availableBytes > 100);
    };

    // clear some logs folder and try again
    if (!hasFreeBytes()) {
        AppUtilities::removeContentDirectory("/mnt/log/log/");
        AppUtilities::removeContentDirectory("/mnt/log/networkLogs/");

        if (!hasFreeBytes()) {
            if (showAlert) emit logAlert("The Logging directory has no space.");
            return "";
        }
    }

    QString filename = "/mnt/log/log/" + QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmss")
                       + ".log";

    SYS_LOG << "generating log file in " << filename;

    // Create log
    auto exitCode = QProcess::execute("/bin/bash", {"-c", "journalctl -u appStherm > " + filename});
    if (exitCode != 0)
    {
        bool removed = QFile::remove(filename); // to ensure no residue is remained

        QString error("Unable to create log file.");
        SYS_LOG << error << exitCode << removed;
        if (showAlert) emit logAlert(error);

        return "";
    }

    return filename;
}

void NUVE::System::sendFirstRunLogFile(bool showAlert)
{
    static int tryWithoutSN = 0;
    tryWithoutSN++;

    if (tryWithoutSN < 3) {
        qWarning() << "Preventing to send too much logs";
        return;

    } else {
        tryWithoutSN = 0;
        TRACE << "Sending first run log file...";
    }

    auto filename = generateLog(showAlert);
    if (filename.isEmpty())
        return;

    auto sendCallback = [=](QString error) {
        auto role = mLogSender.property("role").toString();
        TRACE_CHECK(role != "sendFirstRunLog") << "role seems invalid" << role;

        if (!error.isEmpty()) {
            error = "error while sending first run log directory on remote: " + error;
            qWarning() << error;
            if (showAlert) emit logAlert(error);
            return;
        }

        if (showAlert) emit logAlert("Log is sent for first run error!");
    };

    mLogSender.setRole("sendFirstRunLog", sendCallback);

    auto logRemotePath = mLogRemoteFolderUID + QString("/%0_%1").arg(QString::fromStdString(mUID), filename.split("/").last());

    // Copy file to remote path, should be execute detached but we should prevent a new one before current one finishes
    QString copyFile = QString("sshpass -p '%1' scp  -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2 %3@%4:%5").
                       arg(m_logPassword, filename, m_logUsername, m_logServerAddress, logRemotePath);
    TRACE << "sending log to server " << logRemotePath;
    mLogSender.start("/bin/bash", {"-c", copyFile});
}

bool NUVE::System::sendLogFile(bool showAlert)
{
    auto filename = generateLog(showAlert);
    if (filename.isEmpty()) {
        if (mLastReceivedCommands.contains(Cmd_PushLogs)) {
            SYS_LOG << "Log file generation failed. Command cleared" << Cmd_PushLogs;
            mLastReceivedCommands.remove(Cmd_PushLogs);
        }

        if (showAlert) emit logPrepared(false);
        return false;
    }

    if (showAlert) emit logPrepared(true);

    return sendLogToServer(QStringList(filename), showAlert, true, false, true);
}

void NUVE::System::sendResultsFile(const QString &filepath,
                                   const QString &remoteIP,
                                   const QString &remoteUser,
                                   const QString &remotePassword,
                                   const QString &destination)
{
    auto sendFileCallback = [=](QString error) {
        auto role = mFileSender.property("role").toString();
        TRACE_CHECK(role != "resultsFile") << "role seems invalid" << role;

        if (!error.isEmpty()) {
            error = "error while sending file to remote" + error;
            qWarning() << error;
            emit testPublishFinished(error);
            return;
        }

        TRACE << "file has been sent to remote";
        emit testPublishFinished();
    };

    mFileSender.setRole("resultsFile", sendFileCallback);

    TRACE << "sending file to remote " << destination;

    // Copy file to remote path, should be execute detached but we should prevent a new one before current one finishes
    //! timeout set to 10 seconds as the file is pretty small and should be copied locally to prevent long waiting
    QString copyFile = QString("sshpass -p '%1' scp  -o \"ConnectTimeout=10\" -o \"UserKnownHostsFile=/dev/null\" -o "
                               "\"StrictHostKeyChecking=no\" \"%2\" %3@%4:%5")
                           .arg(remotePassword, filepath, remoteUser, remoteIP, destination);
    mFileSender.start("/bin/bash", {"-c", copyFile});
}

void NUVE::System::startAutoSendLogTimer(int interval)
{
    if (mFirstLogSent) {
        stopAutoSendLogTimer();
        return;
    }

    if (mAutoSendLogtimer == nullptr) {
        mAutoSendLogtimer = new QTimer(this);

        mAutoSendLogtimer->setSingleShot(true);
        mAutoSendLogtimer->callOnTimeout([this]() {
            SYS_LOG << "Sending log automatically: Start sending";
            bool result = this->sendLog(false);

            if (result == true) {
                SYS_LOG << "Sending log automatically: Sending...";
                stopAutoSendLogTimer();

            } else {
                SYS_LOG << "Sending log automatically: Log Send Failed.";

                SYS_LOG << "Sending log automatically: Next attempt in 1 minute.";
                startAutoSendLogTimer(1 * 60 * 1000); // 1 Minute
            }
        });
    }

    if (mAutoSendLogtimer)
        mAutoSendLogtimer->start(interval);
}

void NUVE::System::stopAutoSendLogTimer()
{
    SYS_LOG << "Sending log automatically: Stopping timer";
    if (mAutoSendLogtimer == nullptr) {
        return;
    }

    mAutoSendLogtimer->stop();
    mAutoSendLogtimer->disconnect();
    mAutoSendLogtimer->deleteLater();
    mAutoSendLogtimer = nullptr;
}

void NUVE::System::generateInstallLog()
{
    if (mLogSender.busy()){
        QString error("Previous session is in progress.");
        SYS_LOG << error << "State is :" << mLogSender.state() << mLogSender.keys();

        emit installLogSent(false);
        return;
    }

    if (!checkSendLog(false)) {
        emit installLogSent(false);
        return;
    }

    QJsonObject log;
    QString networkLog = "No Wifi Connection";
    auto wifiConnection = NetworkInterface::me()->connectedWifi();
    if(wifiConnection != nullptr)
        networkLog = wifiConnection->wifiInformation();

    log["ConnetedNetwork"] = networkLog;

    log["iw"] = NetworkInterface::me()->refreshWiFiResult();

    auto lastSettingsResponseData =  mSync->lastSettingsResponseData();
    if (lastSettingsResponseData.empty()) {
        emit installLogSent(false);
        return;
    }

    log["updateData"] = lastSettingsResponseData;

    QJsonDocument jsonDoc(log);
    QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);

    QString filename = "/mnt/log/log/install" +  QDateTime::currentDateTimeUtc().toString("ddMMyyyy") + ".log";

    QFile logFile(filename);
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Unable to open log file:" << filename;
        emit installLogSent(false);
        return;
    }

    QTextStream out(&logFile);
    out << jsonString;

    logFile.close();

    SYS_LOG << "Log written to " << filename;

    auto initialized = mLogSender.property("initialized");
    if (initialized.isValid() && initialized.toBool()) {
        sendLogToServer({filename}, false, false, true, true);

    } else {
        auto dirCreatorCallback = [=](QString error) {
            auto role = mLogSender.property("role").toString();
            TRACE_CHECK(role != "dirLog") << "role seems invalid" << role;

            if (!error.isEmpty()) {
                error = "error while creating log directory on remote: " + error;
                qWarning() << error;
                emit installLogSent(false);
                return;
            }

            SYS_LOG << "Folder created in server successfully";
            mLogSender.setProperty("initialized", true);

            sendLogToServer({filename}, false, false, true, true);
        };

        mLogSender.setRole("dirLog", dirCreatorCallback);

        prepareLogDirectory(dirCreatorCallback);
    }
}

bool NUVE::System::attemptToRunCommand(const QString& command, const QString& tag)
{
    if (mLastReceivedCommands.contains(command) && mLastReceivedCommands[command] == tag) {
        return false;
    }

    bool isApplied = false;
    SYS_LOG << "Attempting command" << command << tag;

    if (command == Cmd_PushLogs) {
        mNetworkLogShouldSend = true;

        if (isBusylogSender()) {
            SYS_LOG << "Log-sender is busy at this momemnt";
        } else {
            SYS_LOG << "Applying" << command << tag;
            if (sendLog(false)) {
                mLastReceivedCommands[command] = tag;
            }
            else {
                SYS_LOG << "Command failed" <<command << tag;
            }
        }
    } else if (command == Cmd_PerfTest) {
        SYS_LOG << "Applying" << command << tag;
        if (PerfTestService::me()->checkTestEligibilityManually("Command")) {
            mLastReceivedCommands[command] = tag;
        }
        else {
            SYS_LOG <<"Command failed" << command << tag;
        }
    } else if (command == Cmd_Reboot) {
        SYS_LOG << "Applying" << command << tag;
        mLastReceivedCommands[command] = tag;
        {QSettings settings; settings.setValue(Key_LastRebootAt, tag);}
        rebootDevice();
    } else if (command == Cmd_PushLiveData) {
        SYS_LOG << "Applying" << command << tag;
        ProtoDataManager::me()->sendDataToServer();
        isApplied = true;
    } else if (command == Cmd_ForgetDevice) {
        SYS_LOG << "Applying" << command << tag;
        mLastReceivedCommands[command] = tag;

        auto callback = [this] (bool success, const QJsonObject& data) {
            SYS_LOG << "forget ended with success:" << success << "Data: " << data;

            // will reboot at the end of process with a timer popup
            emit forgetDeviceRequested();

            mLastReceivedCommands.remove(Cmd_ForgetDevice);
            SYS_LOG << "Reporting Forget success. Command cleared" << Cmd_ForgetDevice;
        };

        mSync->reportCommandResponse(callback, Cmd_ForgetDevice, Cmd_ForgetDeviceResponse, 5);
    }


    return isApplied || (mLastReceivedCommands.contains(command) && mLastReceivedCommands[command] == tag);
}

void NUVE::senderProcess::initialize(std::function<void (QString)> errorHandler, const QString &subject, const QString &joiner)
{
    mErrorHandler = errorHandler;
    mSubject = subject;
    mJoiner = joiner;

    connect(this, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        auto role = property("role").toString();

        QString errorStr = readAllStandardError();
        qWarning() << role << mSubject + ": process has encountered an error:" << error << errorStr;
        errorStr = QString("%1: %0").arg(errorStr).arg(error);

        if (!mCallbacks.contains(role)) {
            QString error = mSubject + ": Callback not found for role " + role;
            TRACE << error;
            mErrorHandler(error);
            return;
        }

        auto callback = mCallbacks.take(role);
        if (callback) {
            callback(errorStr);

        } else {
            QString error = mSubject + ": Callback not valid for role " + role;
            TRACE << error;
            mErrorHandler(error);
        }

    });
    connect(this, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        auto role = property("role").toString();

        QString errorStr;
        if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
            errorStr = readAllStandardOutput() + mJoiner
                       + readAllStandardError();
            qWarning() << role << mSubject + ": process did not exit cleanly" << exitCode
                       << exitStatus << errorStr;
            errorStr = QString("%1-%2: %0").arg(errorStr).arg(exitCode).arg(exitStatus);
        }

        if (!mCallbacks.contains(role)) {
            QString error = mSubject + ": Callback not found for role " + role;
            TRACE << error;
            mErrorHandler(error);
            return;
        }

        auto callback = mCallbacks.take(role);
        if (callback) {
            callback(errorStr);

        } else {
            QString error = mSubject + ": Callback not valid for role " + role;
            TRACE << error;
            mErrorHandler(error);
        }
    });
}

void NUVE::System::setFactoryTestMode(bool newFactoryTestMode)
{
    mFactoryTestMode = newFactoryTestMode;
}

void NUVE::System::saveNetworkLogs() {
    static int networkLogCounter = 0;
    networkLogCounter += 1;
    SYS_LOG << "Save network log called for " << networkLogCounter;

    // disabled for now to prevent filling up the disc
    // TODO: add file management to remove excessive files for enabling this
    return;

    auto generatedFilename = generateLog();
    if (generatedFilename.isEmpty())
        return;

    auto filename = generatedFilename;
    auto newFileName = "/mnt/log/networkLogs/network_" + filename.remove("/mnt/log/log/");
    if (!QFile::copy(generatedFilename, newFileName))
        qWarning() << "unable to copy logs to network folder";
}

bool NUVE::System::sendNetworkLogs() {
    if (!mNetworkLogShouldSend) {
        SYS_LOG << "Network logs are disabled for scaling the cloud. will be able to send if "
                   "receive send log command.";
        return false;
    }

    if (mLogSender.busy()) {
        QString error("Previous session is in progress.");
        SYS_LOG << error << "State is :" << mLogSender.state() << mLogSender.keys();
        return true;
    }

    bool showAlert = false;

    if (!checkSendLog(showAlert)) {
        return false;
    }

    SYS_LOG << "checking for network logs";

    QDir dir("/mnt/log/networkLogs/");
    if (!dir.exists())
        return false;

    // Get a list of all files in the specified path
    QStringList fileList = dir.entryList(QDir::Files);

    if (fileList.isEmpty())
        return false;

    QStringList absFileList;
    // Get absolute file paths
    for (const QString& fileName : fileList) {
        QString absoluteFilePath = dir.absoluteFilePath(fileName);
        absFileList.append(absoluteFilePath);
    }

    if (absFileList.isEmpty()) {
        TRACE << "network log sending set to false as there is no log.";
        mNetworkLogShouldSend = false;
    }

    auto initialized = mLogSender.property("initialized");
    if (initialized.isValid() && initialized.toBool()) {
        sendLogToServer(absFileList, false);

    } else {
        auto dirCreatorCallback = [=](QString error) {
            auto role = mLogSender.property("role").toString();
            TRACE_CHECK(role != "dirLog") << "role seems invalid" << role;

            if (!error.isEmpty()) {
                error = "error while creating log directory on remote: " + error;
                qWarning() << error;
                if (showAlert) emit logAlert(error);
                return;
            }

            SYS_LOG << "Folder created in server successfully";
            mLogSender.setProperty("initialized", true);

            sendLogToServer(absFileList, false);
        };

        mLogSender.setRole("dirLog", dirCreatorCallback);

        prepareLogDirectory(dirCreatorCallback);
    }

    return true;
}

bool NUVE::System::sendLogToServer(const QStringList &filenames,
                                   const bool &showAlert,
                                   bool isRegularLog,
                                   bool isInstallLog,
                                   bool deleteOnFail)
{
    auto sendCallback = [=](QString error) {
        auto role = mLogSender.property("role").toString();
        TRACE_CHECK(role != "sendLog") << "role seems invalid" << role;

        bool isSuccess = error.isEmpty();
        if (isSuccess) {
            if (isRegularLog && mLastReceivedCommands.contains(Cmd_PushLogs)) {
                SYS_LOG << "Reporting" << Cmd_PushLogs;
                auto callback = [this] (bool success, const QJsonObject& data) {
                    mLastReceivedCommands.remove(Cmd_PushLogs);
                    SYS_LOG << "Log sending success. Command cleared" << Cmd_PushLogs;
                };
                mSync->reportCommandResponse(callback, Cmd_PushLogs, "log_sent");
            }

            if (showAlert) emit logSentSuccessfully();

            if (isRegularLog) mFirstLogSent = true;

        } else {
            if (isRegularLog && mLastReceivedCommands.contains(Cmd_PushLogs)) {
                SYS_LOG << "Log sending failed. Command cleared" << Cmd_PushLogs;
                mLastReceivedCommands.remove(Cmd_PushLogs);
            }

            error = "error while sending log directory on remote: " + error;
            qWarning() << error;
            if (showAlert) emit logAlert(error);
        }

        // Delete logs that have been sent to the server or if should be deleted even on Fail.
        if (isSuccess || deleteOnFail) {
            foreach (auto file, filenames) {
                if (!QFile::remove(file))
                    qWarning() << "Could not remove the file: " << file;
            }
        }

        if (isInstallLog)
            emit installLogSent(isSuccess);

        startAutoSendLogTimer(1 * 60 * 1000);
    };

    mLogSender.setRole("sendLog", sendCallback);

    // Copy file to remote path, should be execute detached but we should prevent a new one before current one finishes
    QString copyFile = QString("sshpass -p '%1' scp  -o \"UserKnownHostsFile=/dev/null\" -o \"StrictHostKeyChecking=no\" %2 %3@%4:%5").
                       arg(m_logPassword, filenames.join(" "), m_logUsername, m_logServerAddress, mLogRemoteFolder);
    SYS_LOG << "sending log to server " << mLogRemoteFolder;
    mLogSender.start("/bin/bash", {"-c", copyFile});

    return true;
}

bool NUVE::System::checkSendLog(bool showAlert)
{
    if (!installSSHPass()){
        QString error("Device is not ready to send log!");
        qWarning() << error;
        if (showAlert) emit logAlert(error);
        return false;
    }

    return true;
}

bool NUVE::System::isValidNetworkRequestRestart()
{
    QSettings settings;
    int networkRequestRestartTimes = settings.value(m_NetworkRequestRestartSetting, 0).toInt();

    return networkRequestRestartTimes < 1;
}

void NUVE::System::saveNetworkRequestRestart()
{
    QSettings settings;
    int networkRequestRestartTimes = settings.value(m_NetworkRequestRestartSetting, 0).toInt() + 1;

    settings.setValue(m_NetworkRequestRestartSetting, networkRequestRestartTimes);
}

QStringList NUVE::System::usedDirectories() const
{
    return mUsedDirectories;
}

NUVE::StorageMonitor::StorageMonitor(QObject *parent)
    : QObject(parent)
    , minFreeSpaceRequired(100)
{
    // Set up a timer to check storage periodically
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &StorageMonitor::checkStorageSpace);
    timer->start(5 * 60 * 1000); // Check every 10 seconds
}

void NUVE::StorageMonitor::checkStorageSpace()
{
    QStorageInfo storageInfoExpansion("/mnt/log");

    if (storageInfoExpansion.isValid() && storageInfoExpansion.isReady()) {
        uint64_t freeSpaceMB = storageInfoExpansion.bytesAvailable() / 1024 / 1024;

        if (freeSpaceMB < minFreeSpaceRequired) {
            SYS_LOG << "Low disk space! Taking action. size in MB: " << freeSpaceMB;
            emit lowStorageDetected();
        }
    } else {
        SYS_LOG << "Failed to query storage information.";
    }

    QStorageInfo storageInfoMain("/");

    if (storageInfoMain.isValid() && storageInfoMain.isReady()) {
        uint64_t freeSpaceMB = storageInfoMain.bytesFree() / 1024 / 1024;

        if (freeSpaceMB < 40) {
            SYS_LOG << "Low disk space! Taking action. size in MB: " << freeSpaceMB;
            emit lowSpaceDetected();
        }
    } else {
        SYS_LOG << "Failed to query space information.";
    }
}

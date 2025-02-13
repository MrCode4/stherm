#include "ProtoDataManager.h"

#include "Config.h"
#include "LogHelper.h"
#include "AppUtilities.h"
#include "DeviceInfo.h"
#include "NetworkInterface.h"

#ifdef PROTOBUF_ENABLED
#include <ctime>
#include <fstream>
#include <google/protobuf/util/time_util.h>
#endif

const QString m_binaryFilesPath          = PROTOBUFF_FILES_PATH;
const int     m_memoryLimitationRecords = 3000;
const int     m_maximumProtoFolderSize = 50 * 1024 * 1024; // 50 MB
const int     m_minFileSizeToRemove       = 5 * 1024 * 1024;  // 5 MB

const double  m_temperatureThreshold    = 1 / 1.8;
const double  m_mcuTemperatureThreshold = 3 / 1.8;

Q_LOGGING_CATEGORY(ProtobufferDataManager, "ProtobufferDataManager")
#define PROTO_LOG TRACE_CATEGORY(ProtobufferDataManager)

#ifdef PROTOBUF_ENABLED
using google::protobuf::util::TimeUtil;
#endif

ProtoDataManager* ProtoDataManager::mMe = nullptr;

ProtoDataManager* ProtoDataManager::me()
{
    if (!mMe) mMe = new ProtoDataManager(qApp);

    return mMe;
}

ProtoDataManager::ProtoDataManager(QObject *parent)
    : mSendingToServer{false}
    , DevApiExecutor{parent}
{
#ifdef PROTOBUF_ENABLED
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    mLateastDataPoint = new LiveDataPoint();

    mSenderTimer.setInterval(30 * 60 * 1000);
    mSenderTimer.setSingleShot(false);
    connect(&mSenderTimer, &QTimer::timeout, this, [this]() {
        sendDataToServer();
    });

    mSenderTimer.start();

    mDataPointLogger.setInterval(1000);
    mDataPointLogger.setSingleShot(true);
    connect(&mDataPointLogger, &QTimer::timeout, this, [this]() {
        logStashData();
    });

    mCreateGeneralBufferTimer.setInterval(1 * 60 * 60 * 1000);
    connect(&mCreateGeneralBufferTimer, &QTimer::timeout, this, [this]() {
        sendFullDataPacketToServer();
    });
    mCreateGeneralBufferTimer.start();

    mChangeMode = CMNone;

    connect(NetworkInterface::me(), &NetworkInterface::hasInternetChanged, this, [this]() {
        if (NetworkInterface::me()->hasInternet()) {
            sendDataToServer();
        }
    });

    // Send the old data to server.
    sendDataToServer();
#endif
}

ProtoDataManager::~ProtoDataManager()
{
#ifdef PROTOBUF_ENABLED
    generateBinaryFile();

    mDataPointLogger.stop();
    mSenderTimer.stop();
    mCreateGeneralBufferTimer.stop();

    mLateastDataPoint->Clear();
    delete mLateastDataPoint;

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
#endif
}

void ProtoDataManager::sendDataToServer()
{
#ifdef PROTOBUF_ENABLED
    if (!NetworkInterface::me()->hasInternet()) {
        return;
    }

    if (Device->serialNumber().isEmpty()) {
        return;
    }

    auto protoFoldersize = AppUtilities::getFolderUsedBytes(m_binaryFilesPath);
    bool existValidBinaryFile = protoFoldersize > 0;

    if (mLiveDataPointList.data_points_size() < 1 && !existValidBinaryFile) {
        PROTO_LOG << "No data for sending.";
        return;
    }

    if (!existValidBinaryFile) {
        PROTO_LOG << "Sending data points: " << mLiveDataPointList.data_points_size();
        generateBinaryFile();

    } else {
        PROTO_LOG << "Sending old data points in the file with size (Bytes): " << protoFoldersize;
    }

    mSendingToServer = true;
    mDataPointLogger.stop();

    QByteArray serializedData = readBinaryFiles();

    auto url = baseUrl() + QString("api/monitor/data?sn=%0").arg(Device->serialNumber());
    auto callback = [this] (QNetworkReply* reply, const QByteArray &rawData, QJsonObject &data) {
        // ServiceUnavailableError: 403
        if (reply->error() == QNetworkReply::NoError || reply->error() == QNetworkReply::ServiceUnavailableError) {
            PROTO_LOG << " files sent, remove: " << AppUtilities::removeContentDirectory(m_binaryFilesPath);

        } else {
            qWarning() << reply->url() << " - ERROR: " << reply->errorString();
        }

        mSendingToServer = false;
        mDataPointLogger.start();
    };

    callPostApi(url, serializedData, callback, true, "application/x-protobuf");
#endif
}

QByteArray ProtoDataManager::readBinaryFiles() {
    QByteArray serializedData;

    QDir protoDir(m_binaryFilesPath);
    QFileInfoList fileList = protoDir.entryInfoList({"*.bin"}, QDir::Files, QDir::Time);

    if (fileList.isEmpty()) {
        PROTO_LOG << "File list is empty!";
        return serializedData;
    }

    while (!fileList.isEmpty()) {
        QFile file(fileList.last().absoluteFilePath());
        if (file.exists() && file.open(QIODevice::ReadOnly)) {
            serializedData += file.readAll();
            file.close();

        } else {
            qWarning() << "Error opening file: " << file.errorString() << " - Remove: "
                       <<file.remove();
        }

        fileList.removeLast();
    }

    QString fileName = QString("%0/%1.bin").arg(m_binaryFilesPath, QString::number(QDateTime::currentDateTime().currentMSecsSinceEpoch()));
    QFile newFile(fileName);
    if (newFile.open(QIODevice::WriteOnly)) {
        auto writtenDataSize = newFile.write(serializedData);
        if (writtenDataSize == serializedData.size()) {
            PROTO_LOG << " files sent, remove files due to file accumulation: " << AppUtilities::removeContentDirectory(m_binaryFilesPath);

        } else {
            newFile.remove();
        }

        newFile.close();
    } else {
        qWarning() << "[ProtoDataManager] could not accumulate the files.";
        newFile.remove();
    }


    return serializedData;
}

void ProtoDataManager::sendFullDataPacketToServer()
{
    // Restart the timer
    mCreateGeneralBufferTimer.start();

    updateChangeMode(CMAll);
    logStashData();
}

void ProtoDataManager::generateBinaryFile()
{
#ifdef PROTOBUF_ENABLED
    if (mSendingToServer) {
        return;
    }

    if (mLiveDataPointList.data_points_size() < 1) {
        return;
    }

    checkMemoryAndCleanup();

    QString fileName = QString("%0/%1.bin").arg(m_binaryFilesPath, QString::number(QDateTime::currentDateTime().currentMSecsSinceEpoch()));
    std::fstream output(fileName.toStdString(), std::ios::out | std::ios::binary);

    if (mLiveDataPointList.SerializeToOstream(&output)) {
        mLiveDataPointList.clear_data_points();
        output.close();

    } else {
        output.close();
        QFile::remove(fileName);
    }

#endif
}

void ProtoDataManager::checkMemoryAndCleanup()
{
    auto protoFoldersize = AppUtilities::getFolderUsedBytes(m_binaryFilesPath);
    QDir protoDir(m_binaryFilesPath);
    QFileInfoList fileList = protoDir.entryInfoList(QDir::Files, QDir::Time);

    if (protoFoldersize > m_maximumProtoFolderSize) {
        PROTO_LOG << m_binaryFilesPath << " size (Bytes): " <<  protoFoldersize;

        qint64 clearedSize = 0;

        // When the maximum size is reached, we need to free up at least 5MB of space.
        while (!fileList.isEmpty()) {
            auto file = fileList.last();

            auto fileToRemove = file.absoluteFilePath();
            if (QFile::remove(fileToRemove)) {
                clearedSize += file.size();

            } else {
                PROTO_LOG << QString("Error removing file %1.").arg(fileToRemove);
            }

            if (clearedSize >= m_minFileSizeToRemove) {
                break;
            }

            fileList.removeLast();
        }

        PROTO_LOG << QString("Removed some files due to memory limitation (cleared size: %0 Bytes)").arg(QString::number(clearedSize));

        protoFoldersize = AppUtilities::getFolderUsedBytes(m_binaryFilesPath);
        PROTO_LOG << m_binaryFilesPath << "  size after cleanUp (Bytes): " <<  protoFoldersize;
    }
}

void ProtoDataManager::setSetTemperature(const double &tempratureC)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_set_temperature() &&
        qAbs(mLateastDataPoint->set_temperature() - tempratureC) < m_temperatureThreshold) {
        return;
    }

    mLateastDataPoint->set_set_temperature(tempratureC);
    updateChangeMode(CMSetTemperature);
#endif
}

void ProtoDataManager::setSetHumidity(const double &humidity)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_set_humidity() &&
        qAbs(mLateastDataPoint->set_humidity() - humidity) < 1.0) {
        return;
    }

    mLateastDataPoint->set_set_humidity(humidity);
    updateChangeMode(CMSetHumidity);
#endif
}

void ProtoDataManager::setCurrentTemperature(const double &tempratureC)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_current_temperature_embedded() &&
        qAbs(mLateastDataPoint->current_temperature_embedded() - tempratureC) < m_temperatureThreshold) {
        return;
    }

    mLateastDataPoint->set_current_temperature_embedded(tempratureC);
    updateChangeMode(CMCurrentTemperature);
#endif
}

void ProtoDataManager::setCurrentHumidity(const double &humidity)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_current_humidity_embedded() &&
        qAbs(mLateastDataPoint->current_humidity_embedded() - humidity) < 1.0) {
        return;
    }

    mLateastDataPoint->set_current_humidity_embedded(humidity);
    updateChangeMode(CMCurrentHumidity);
#endif
}

void ProtoDataManager::setMCUTemperature(const double &mcuTempratureC)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_current_temperature_mcu() &&
        qAbs(mLateastDataPoint->current_temperature_mcu() - mcuTempratureC) <  m_mcuTemperatureThreshold) {
        return;
    }

    mLateastDataPoint->set_current_temperature_mcu(mcuTempratureC);
    updateChangeMode(CMMCUTemperature);
#endif
}

void ProtoDataManager::setAirPressure(const int &airPressureHPa)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_air_pressure_embedded() &&
        qAbs(mLateastDataPoint->air_pressure_embedded() - airPressureHPa) < 1.0) {
        return;
    }

    mLateastDataPoint->set_air_pressure_embedded(airPressureHPa);
    updateChangeMode(CMAirPressure);
#endif
}

void ProtoDataManager::setCurrentAirQuality(const int &airQuality)
{
#ifdef PROTOBUF_ENABLED
    const AirQuality airQualityE = (AirQuality)(airQuality + 1);
    if (mLateastDataPoint->has_current_air_quality() &&
        mLateastDataPoint->current_air_quality() == airQualityE) {
        return;
    }

    mLateastDataPoint->set_current_air_quality(airQualityE);
    updateChangeMode(CMCurrentAirQuality);
#endif
}

void ProtoDataManager::setCurrentCoolingStage(const int &coolingStage)
{
#ifdef PROTOBUF_ENABLED
    const CoolingStage coolingStageE = (CoolingStage)coolingStage;
    if (mLateastDataPoint-> has_current_cooling_stage() &&
        mLateastDataPoint->current_cooling_stage() == coolingStageE) {
        return;
    }

    mLateastDataPoint->set_current_cooling_stage(coolingStageE);
    updateChangeMode(CMCurrentCoolingStage);
#endif
}

void ProtoDataManager::setCurrentHeatingStage(const int &heatingStage)
{
#ifdef PROTOBUF_ENABLED
    const HeatingStage heatingStageE = (HeatingStage)heatingStage;
    if (mLateastDataPoint->has_current_heating_stage() &&
        mLateastDataPoint->current_heating_stage() == heatingStageE) {
        return;
    }

    mLateastDataPoint->set_current_heating_stage(heatingStageE);
    updateChangeMode(CMCurrentHeatingStage);
#endif
}

void ProtoDataManager::setCurrentFanStatus(const bool &fanStatus)
{
#ifdef PROTOBUF_ENABLED
    const FanStatus fanStatusE = (FanStatus)(fanStatus ? 1 : 0);
    if (mLateastDataPoint->has_current_fan_status() &&
        mLateastDataPoint->current_fan_status() == fanStatusE) {
        return;
    }

    mLateastDataPoint->set_current_fan_status(fanStatusE);
    updateChangeMode(CMCurrentFanStatus);
#endif
}

void ProtoDataManager::setSystemType(const AppSpecCPP::SystemType &systemSetup)
{
#ifdef PROTOBUF_ENABLED
    const auto sysTypeStr = AppSpecCPP::systemTypeString(systemSetup, true).toStdString();
    if (mLateastDataPoint->has_system_type() &&
        mLateastDataPoint->system_type() == sysTypeStr) {
        return;
    }

    mLateastDataPoint->set_system_type(sysTypeStr);
    updateChangeMode(CMSystemType);
#endif
}

void ProtoDataManager::setRunningMode(const AppSpecCPP::SystemMode &runningMode)
{
#ifdef PROTOBUF_ENABLED
    const auto runningModeStr = AppSpecCPP::systemModeToString(runningMode, false).toStdString();
    if (mLateastDataPoint->has_running_mode() &&
        mLateastDataPoint->running_mode() == runningModeStr) {
        return;
    }

    mLateastDataPoint->set_running_mode(runningModeStr);
    updateChangeMode(CMRunningMode);
#endif
}

void ProtoDataManager::setOnlineStatus(const bool &onlineStatus)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_online_status() &&
        mLateastDataPoint->online_status() == onlineStatus) {
        return;
    }

    mLateastDataPoint->set_online_status(onlineStatus);
    updateChangeMode(CMOnlineStatus);
#endif
}

void ProtoDataManager::setLedStatus(const bool &ledStatus)
{
#ifdef PROTOBUF_ENABLED
    const LedStatus ledStatusE = (LedStatus)(ledStatus ? 1 : 0);
    if (mLateastDataPoint->has_led_status() &&
        mLateastDataPoint->led_status() == ledStatusE) {
        return;
    }

    mLateastDataPoint->set_led_status(ledStatusE);
    updateChangeMode(CMLedStatus);
#endif
}

#ifdef PROTOBUF_ENABLED
LiveDataPoint *ProtoDataManager::addNewPoint()
{
    if (mLiveDataPointList.data_points_size() > 0 && mLiveDataPointList.data_points_size() > m_memoryLimitationRecords) {
        int outRangeIndex = mLiveDataPointList.data_points_size() - m_memoryLimitationRecords;
        PROTO_LOG << "Delete the first data points in the list due to memory limitation."
        << ", Deleted range: 0 to " << outRangeIndex << mLiveDataPointList.data_points_size();
        mLiveDataPointList.mutable_data_points()->DeleteSubrange(0, outRangeIndex);
    }

    auto newPoint = mLiveDataPointList.add_data_points();

    // Update time
    *newPoint->mutable_time() = TimeUtil::SecondsToTimestamp(time(NULL));

    return newPoint;
}
#endif

void ProtoDataManager::logStashData()
{
#ifdef PROTOBUF_ENABLED
    if (mChangeMode != CMNone) {
        auto newPoint = addNewPoint();

        PROTO_LOG << "New point created due to " << mChangeMode;

        if (mChangeMode & CMSetTemperature) {
            newPoint->set_set_temperature(mLateastDataPoint->set_temperature());
        }
        if (mChangeMode & CMSetHumidity) {
            newPoint->set_set_humidity(mLateastDataPoint->set_humidity());
        }
        if (mChangeMode & CMCurrentTemperature) {
            newPoint->set_current_temperature_embedded(mLateastDataPoint->current_temperature_embedded());
        }
        if (mChangeMode & CMCurrentHumidity) {
            newPoint->set_current_humidity_embedded(mLateastDataPoint->current_humidity_embedded());
        }
        if (mChangeMode & CMMCUTemperature) {
            newPoint->set_current_temperature_mcu(mLateastDataPoint->current_temperature_mcu());
        }
        if (mChangeMode & CMAirPressure) {
            newPoint->set_air_pressure_embedded(mLateastDataPoint->air_pressure_embedded());
        }
        if (mChangeMode & CMCurrentAirQuality) {
            newPoint->set_current_air_quality(mLateastDataPoint->current_air_quality());
        }
        if (mChangeMode & CMCurrentCoolingStage) {
            newPoint->set_current_cooling_stage(mLateastDataPoint->current_cooling_stage());
        }
        if (mChangeMode & CMCurrentHeatingStage) {
            newPoint->set_current_heating_stage(mLateastDataPoint->current_heating_stage());
        }
        if (mChangeMode & CMCurrentFanStatus) {
            newPoint->set_current_fan_status(mLateastDataPoint->current_fan_status());
        }
        if (mChangeMode & CMLedStatus) {
            newPoint->set_led_status(mLateastDataPoint->led_status());
        }
        if (mChangeMode & CMSystemType) {
            newPoint->set_system_type(mLateastDataPoint->system_type());
        }
        if (mChangeMode & CMRunningMode) {
            newPoint->set_running_mode(mLateastDataPoint->running_mode());
        }
        if (mChangeMode & CMOnlineStatus) {
            newPoint->set_online_status(mLateastDataPoint->online_status());
        }

        newPoint->set_is_sync(mChangeMode == CMAll);

        updateChangeMode(CMNone);
        generateBinaryFile();
    }
#endif
}

void ProtoDataManager::updateChangeMode(ChangeMode cm)
{
#ifdef PROTOBUF_ENABLED
    if (cm == CMNone) {
        mChangeMode = cm;
        mDataPointLogger.stop();
        return;
    }

    if (cm == CMAll)
        mChangeMode = cm;
    else if ((mChangeMode != CMAll) && (mChangeMode & cm) == 0)
        mChangeMode |= cm;

    if (!mDataPointLogger.isActive())
        mDataPointLogger.start();
#endif
}

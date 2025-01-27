#include "ProtoDataManager.h"

#include "Config.h"
#include "LogHelper.h"

#ifdef PROTOBUF_ENABLED
#include "DeviceInfo.h"
#include <ctime>
#include <fstream>
#include <google/protobuf/util/time_util.h>
#endif

const QString BINARYFILEPATH         = PROTOBUFF_FILE_PATH;
const int     MEMORYLIMITAIONRECORDS = 3000;
const double  TEMPERATURETHRESHOLD   = 1 / 1.8;

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
    : DevApiExecutor{parent}
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

    mDataPointLogger.setInterval(30 * 1000);
    mDataPointLogger.setSingleShot(true);
    connect(&mDataPointLogger, &QTimer::timeout, this, [this]() {
        logStashData();
    });

    mCreatGeneralBufferTimer.setInterval(1 * 60 * 60 * 1000);
    connect(&mCreatGeneralBufferTimer, &QTimer::timeout, this, [this]() {
        updateChangeMode(CMAll);
        logStashData();
    });
    mCreatGeneralBufferTimer.start();

    mChangeMode = CMNone;

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
    mCreatGeneralBufferTimer.stop();

    delete mLateastDataPoint;
    mLateastDataPoint->Clear();

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
#endif
}

void ProtoDataManager::sendDataToServer()
{
#ifdef PROTOBUF_ENABLED
    QFileInfo binFile(BINARYFILEPATH);
    bool existValidBinaryFile = binFile.exists() && binFile.size() > 0;

    if (mLiveDataPointList.data_points_size() < 1 && !existValidBinaryFile) {
        PROTO_LOG << "No data for sending.";

        if (binFile.exists())
           PROTO_LOG << BINARYFILEPATH << " - Invalid file removed: " << QFile::remove(BINARYFILEPATH);

        return;
    }

    if (!existValidBinaryFile) {
        PROTO_LOG << "Sending data points: " << mLiveDataPointList.data_points_size();
        generateBinaryFile();

    } else {
        PROTO_LOG << "Sending old data points in the file: " << existValidBinaryFile;
    }

    QByteArray serializedData;
    QFile file(BINARYFILEPATH);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        TRACE << file.errorString();
        serializedData = file.readAll();
        file.close();

    } else {
        qWarning() << "Error opening file: " << file.errorString();
        return;
    }

    auto url = baseUrl() + QString("api/monitor/data?sn=%0").arg(Device->serialNumber());
    auto callback = [this, existValidBinaryFile] (QNetworkReply* reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {

            if (QFileInfo::exists(BINARYFILEPATH)) {
                PROTO_LOG << BINARYFILEPATH << " file sent, remove: " << QFile::remove(BINARYFILEPATH);
            }

            if(!existValidBinaryFile) {
                mLiveDataPointList.clear_data_points();

            } else if (mLiveDataPointList.data_points_size() > 1) {
                QTimer::singleShot(10000, this, [this] {
                    sendDataToServer();
                });
            }

        } else {
            qWarning() << reply->url() << " - ERROR: " << reply->errorString();
        }

    };

    callPostApi(url, serializedData, callback, true, "application/x-protobuf");
#endif
}

void ProtoDataManager::generateBinaryFile()
{
#ifdef PROTOBUF_ENABLED
    std::fstream output(BINARYFILEPATH.toStdString(), std::ios::out | std::ios::binary);
    mLiveDataPointList.SerializeToOstream(&output);
    output.close();
#endif
}

void ProtoDataManager::setSetTemperature(const double &tempratureC)
{
#ifdef PROTOBUF_ENABLED
    if (mLateastDataPoint->has_set_temperature() &&
        qAbs(mLateastDataPoint->set_temperature() - tempratureC) < TEMPERATURETHRESHOLD) {
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
        qAbs(mLateastDataPoint->current_temperature_embedded() - tempratureC) < TEMPERATURETHRESHOLD) {
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
        qAbs(mLateastDataPoint->current_temperature_mcu() - mcuTempratureC) <  TEMPERATURETHRESHOLD) {
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
    if (mLiveDataPointList.data_points_size() > 0 && mLiveDataPointList.data_points_size() > MEMORYLIMITAIONRECORDS) {
        int outRangeIndex = mLiveDataPointList.data_points_size() - MEMORYLIMITAIONRECORDS;
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

        newPoint->set_is_sync(mChangeMode == CMAll);

        updateChangeMode(CMNone);
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

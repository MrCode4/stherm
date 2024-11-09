#include "ProtoDataManagerCPP.h"

#include <ctime>
#include <fstream>
#include <google/protobuf/util/time_util.h>

#include "DeviceInfo.h"
#include "LogHelper.h"



Q_LOGGING_CATEGORY(ProtobufferDataManager, "ProtobufferDataManager")
#define PROTO_LOG TRACE_CATEGORY(ProtobufferDataManager)

using google::protobuf::util::TimeUtil;

ProtoDataManagerCPP::ProtoDataManagerCPP(QObject *parent)
    : DevApiExecutor{parent}
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    mLateastDataPoint = new LiveDataPoint();

    mSenderTimer.setInterval(30 * 60 * 1000);
    connect(&mSenderTimer, &QTimer::timeout, this, [this]() {
        std::fstream output("/usr/local/bin/output.bin", std::ios::out | std::ios::binary);
        mLiveDataPointList.SerializeToOstream(&output);
        output.close();

        QByteArray serializedData;
        QFile file("/usr/local/bin/output.bin");
        if (file.exists() && file.open(QIODevice::ReadOnly)) {
            TRACE << file.errorString();
            serializedData = file.readAll();
            file.close();
        } else {
            qWarning() << "Error opening file: " << file.errorString();
            return;
        }

        auto url = baseUrl() + QString("api/monitor/data?sn=%0").arg(Device->serialNumber());
        auto callback = [this](QNetworkReply* reply, const QByteArray &rawData, QJsonObject &data) {
            PROTO_LOG << "MAK tersa" << reply->errorString();

        };

        callPostApi(url, serializedData, callback, true, "application/x-protobuf");
    });

    connect(&mCreatGeneralBufferTimer, &QTimer::timeout, this, [this]() {
        auto newPoint = addNewPoint();
    });

    mDataPointLogger.setInterval(30 * 1000);
    connect(&mDataPointLogger, &QTimer::timeout, this, [this]() {
        logStashData();
    });

    mCreatGeneralBufferTimer.setInterval(1 * 60 * 60 * 1000);
    connect(&mCreatGeneralBufferTimer, &QTimer::timeout, this, [this]() {
        updateChangeMode(CMAll);
        logStashData();
    });
}

ProtoDataManagerCPP::~ProtoDataManagerCPP()
{
    logStashData();

    mDataPointLogger.stop();
    mSenderTimer.stop();

    delete mLateastDataPoint;
    mLateastDataPoint->Clear();

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
}

void ProtoDataManagerCPP::updateData() {
    auto callback = [this](QNetworkReply* reply, const QByteArray &rawData, QJsonObject &data) {
        TRACE << "test: " << reply->errorString();

    };

    QByteArray serializedData;
    QFile file("/usr/local/bin/response2.bin");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        serializedData = file.readAll();
        file.close();
    } else {
        qWarning() << "Error opening file: " << file.errorString();
        return;
    }

    QString sn = Device->serialNumber();
    auto url = baseUrl() + QString("api/monitor/data?sn=%0").arg(sn);
    callPostApi(url, serializedData, callback, true, "application/x-protobuf");
}

void ProtoDataManagerCPP::setSetTemperature(const double &temprature)
{
    mLateastDataPoint->set_set_temperature(temprature);
    updateChangeMode(CMSetTemperature);
}

void ProtoDataManagerCPP::setSetHumidity(const double &humidity)
{
    mLateastDataPoint->set_set_humidity(humidity);
    updateChangeMode(CMSetHumidity);
}

void ProtoDataManagerCPP::setCurrentTemperature(const double &temprature)
{
    mLateastDataPoint->set_current_temperature_embedded(temprature);
    updateChangeMode(CMCurrentTemperature);
}

void ProtoDataManagerCPP::setCurrentHumidity(const double &humidity)
{
    mLateastDataPoint->set_current_humidity_embedded(humidity);
    updateChangeMode(CMCurrentHumidity);
}

void ProtoDataManagerCPP::setMCUTemperature(const double &mcuTemprature)
{
    mLateastDataPoint->set_current_temperature_mcu(mcuTemprature);
    updateChangeMode(CMMCUTemperature);
}

void ProtoDataManagerCPP::setAirPressure(const int &airQuality)
{
    mLateastDataPoint->set_air_pressure_embedded(airQuality);
    updateChangeMode(CMAirPressure);
}

void ProtoDataManagerCPP::setCurrentAirQuality(const int &airQuality)
{
    const AirQuality airQualityE = (AirQuality)(airQuality + 1);
    mLateastDataPoint->set_current_air_quality(airQualityE);
    updateChangeMode(CMCurrentAirQuality);
}

void ProtoDataManagerCPP::setCurrentCoolingStage(const int &coolingStage)
{
    const CoolingStage coolingStageE = (CoolingStage)coolingStage;
    mLateastDataPoint->set_current_cooling_stage(coolingStageE);
    updateChangeMode(CMCurrentCoolingStage);
}

void ProtoDataManagerCPP::setCurrentHeatingStage(const bool &heatingStage)
{
    const HeatingStage heatingStageE = (HeatingStage)heatingStage;
    mLateastDataPoint->set_current_heating_stage(heatingStageE);
    updateChangeMode(CMCurrentHeatingStage);
}

void ProtoDataManagerCPP::setCurrentFanStatus(const bool &falStatus)
{
    const FanStatus fanStatusE = (FanStatus)(falStatus ? 1 : 0);
    mLateastDataPoint->set_current_fan_status(fanStatusE);
    updateChangeMode(CMCurrentFanStatus);
}

void ProtoDataManagerCPP::setLedStatus(const bool &ledStatus)
{
    const LedStatus ledStatusE = (LedStatus)(ledStatus ? 1 : 0);
    mLateastDataPoint->set_led_status(ledStatusE);
    updateChangeMode(CMLedStatus);
}

LiveDataPoint *ProtoDataManagerCPP::addNewPoint()
{
    if (mLiveDataPointList.data_points_size() > 3000)
        mLiveDataPointList.mutable_data_points()->DeleteSubrange(0, 1);

    auto newPoint = mLiveDataPointList.add_data_points();

    // Update time
    *newPoint->mutable_time() = TimeUtil::SecondsToTimestamp(time(NULL));

    return newPoint;
}

void ProtoDataManagerCPP::logStashData()
{
    if (changeMode != CMNone) {
        auto newPoint = addNewPoint();

        if (changeMode & CMSetTemperature) {
            newPoint->set_set_temperature(mLateastDataPoint->set_temperature());
        }
        if (changeMode & CMSetHumidity) {
            newPoint->set_set_humidity(mLateastDataPoint->set_humidity());
        }
        if (changeMode & CMCurrentTemperature) {
            newPoint->set_current_temperature_embedded(mLateastDataPoint->current_temperature_embedded());
        }
        if (changeMode & CMCurrentHumidity) {
            newPoint->set_current_humidity_embedded(mLateastDataPoint->current_humidity_embedded());
        }
        if (changeMode & CMMCUTemperature) {
            newPoint->set_current_temperature_mcu(mLateastDataPoint->current_temperature_mcu());
        }
        if (changeMode & CMAirPressure) {
            newPoint->set_air_pressure_embedded(mLateastDataPoint->air_pressure_embedded());
        }
        if (changeMode & CMCurrentAirQuality) {
            newPoint->set_current_air_quality(mLateastDataPoint->current_air_quality());
        }
        if (changeMode & CMCurrentCoolingStage) {
            newPoint->set_current_cooling_stage(mLateastDataPoint->current_cooling_stage());
        }
        if (changeMode & CMCurrentHeatingStage) {
            newPoint->set_current_heating_stage(mLateastDataPoint->current_heating_stage());
        }
        if (changeMode & CMCurrentFanStatus) {
            newPoint->set_current_fan_status(mLateastDataPoint->current_fan_status());
        }
        if (changeMode & CMLedStatus) {
            newPoint->set_led_status(mLateastDataPoint->led_status());
        }

        updateChangeMode(CMNone);
    }
}

void ProtoDataManagerCPP::updateChangeMode(ChangeMode cm)
{
    if (cm == CMNone) {
        changeMode = cm;
        mDataPointLogger.stop();
    }

    changeMode |= cm;
    if (!mDataPointLogger.isActive())
        mDataPointLogger.start();
}

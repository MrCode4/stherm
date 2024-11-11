#include "ProtoDataManagerCPP.h"

#include <ctime>
#include <fstream>
#include <google/protobuf/util/time_util.h>

#include "DeviceInfo.h"
#include "LogHelper.h"

const QString BINPATH = QString ("/usr/local/bin/output.bin");
const int     MEMORYLIMITAIONRECORDS = 3000;

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
        PROTO_LOG << "Sending data points: " << mLiveDataPointList.add_data_points();
        std::fstream output(BINPATH.toStdString(), std::ios::out | std::ios::binary);
        mLiveDataPointList.SerializeToOstream(&output);
        output.close();

        QByteArray serializedData;
        QFile file(BINPATH);
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
            if (reply->error() == QNetworkReply::NoError) {
                mLiveDataPointList.clear_data_points();
                if (QFileInfo::exists(BINPATH)) {
                    PROTO_LOG << BINPATH << "File sent, remove: " << QFile::remove(BINPATH);
                }
            } else {
                qWarning() << "ERROR: " << reply->errorString();
            }

        };

        callPostApi(url, serializedData, callback, true, "application/x-protobuf");
    });

    connect(&mCreatGeneralBufferTimer, &QTimer::timeout, this, [this]() {
        auto newPoint = addNewPoint();
    });

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

    //! TODO
    //! Set default
    setAirPressure(101325);
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

void ProtoDataManagerCPP::setSetTemperature(const double &tempratureC)
{
    if ((mLateastDataPoint->set_temperature() - tempratureC) < 1 / 1.8) {
        return;
    }

    mLateastDataPoint->set_set_temperature(tempratureC);
    updateChangeMode(CMSetTemperature);
}

void ProtoDataManagerCPP::setSetHumidity(const double &humidity)
{
    if ((mLateastDataPoint->set_humidity() - humidity) < 1.0) {
        return;
    }

    mLateastDataPoint->set_set_humidity(humidity);
    updateChangeMode(CMSetHumidity);
}

void ProtoDataManagerCPP::setCurrentTemperature(const double &tempratureC)
{
    if ((mLateastDataPoint->current_temperature_embedded() - tempratureC) < 1 / 1.8) {
        return;
    }

    mLateastDataPoint->set_current_temperature_embedded(tempratureC);
    updateChangeMode(CMCurrentTemperature);
}

void ProtoDataManagerCPP::setCurrentHumidity(const double &humidity)
{
    if ((mLateastDataPoint->current_humidity_embedded() - humidity) < 1.0) {
        return;
    }

    mLateastDataPoint->set_current_humidity_embedded(humidity);
    updateChangeMode(CMCurrentHumidity);
}

void ProtoDataManagerCPP::setMCUTemperature(const double &mcuTempratureC)
{
    if ((mLateastDataPoint->current_temperature_mcu() - mcuTempratureC) < 1.0) {
        return;
    }

    mLateastDataPoint->set_current_temperature_mcu(mcuTempratureC);
    updateChangeMode(CMMCUTemperature);
}

void ProtoDataManagerCPP::setAirPressure(const int &airPressureHPa)
{
    if ((mLateastDataPoint->air_pressure_embedded() - airPressureHPa) < 1.0) {
        return;
    }

    mLateastDataPoint->set_air_pressure_embedded(airPressureHPa);
    updateChangeMode(CMAirPressure);
}

void ProtoDataManagerCPP::setCurrentAirQuality(const int &airQuality)
{
    const AirQuality airQualityE = (AirQuality)(airQuality + 1);
    if (mLateastDataPoint->current_air_quality() == airQualityE) {
        return;
    }

    mLateastDataPoint->set_current_air_quality(airQualityE);
    updateChangeMode(CMCurrentAirQuality);
}

void ProtoDataManagerCPP::setCurrentCoolingStage(const int &coolingStage)
{
    const CoolingStage coolingStageE = (CoolingStage)coolingStage;
    if (mLateastDataPoint->current_cooling_stage() == coolingStageE) {
        return;
    }

    mLateastDataPoint->set_current_cooling_stage(coolingStageE);
    updateChangeMode(CMCurrentCoolingStage);
}

void ProtoDataManagerCPP::setCurrentHeatingStage(const bool &heatingStage)
{
    const HeatingStage heatingStageE = (HeatingStage)heatingStage;
    if (mLateastDataPoint->current_heating_stage() == heatingStageE) {
        return;
    }

    mLateastDataPoint->set_current_heating_stage(heatingStageE);
    updateChangeMode(CMCurrentHeatingStage);
}

void ProtoDataManagerCPP::setCurrentFanStatus(const bool &fanStatus)
{
    const FanStatus fanStatusE = (FanStatus)(fanStatus ? 1 : 0);
    if (mLateastDataPoint->current_fan_status() == fanStatusE) {
        return;
    }

    mLateastDataPoint->set_current_fan_status(fanStatusE);
    updateChangeMode(CMCurrentFanStatus);
}

void ProtoDataManagerCPP::setLedStatus(const bool &ledStatus)
{
    const LedStatus ledStatusE = (LedStatus)(ledStatus ? 1 : 0);
    if (mLateastDataPoint->led_status() == ledStatusE) {
        return;
    }

    mLateastDataPoint->set_led_status(ledStatusE);
    updateChangeMode(CMLedStatus);
}

LiveDataPoint *ProtoDataManagerCPP::addNewPoint()
{
    if (mLiveDataPointList.data_points_size() > 0 && mLiveDataPointList.data_points_size() > MEMORYLIMITAIONRECORDS) {
        PROTO_LOG << "Delete the first data point in the list due to memory limitation. Deleted point time:" << mLiveDataPointList.data_points(0).time().seconds()
        << ", Deleted range: 0-" << (mLiveDataPointList.data_points_size() - MEMORYLIMITAIONRECORDS) << mLiveDataPointList.data_points_size();
        mLiveDataPointList.mutable_data_points()->DeleteSubrange(0, mLiveDataPointList.data_points_size() - MEMORYLIMITAIONRECORDS);
    }

    auto newPoint = mLiveDataPointList.add_data_points();

    // Update time
    *newPoint->mutable_time() = TimeUtil::SecondsToTimestamp(time(NULL));

    return newPoint;
}

void ProtoDataManagerCPP::logStashData()
{
    if (mChangeMode != CMNone) {
        auto newPoint = addNewPoint();

        PROTO_LOG << "New ponit created due to " << mChangeMode;

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

        newPoint->set_is_sync(mChangeMode & CMAll);

        updateChangeMode(CMNone);
    }
}

void ProtoDataManagerCPP::updateChangeMode(ChangeMode cm)
{
    if (cm == CMNone) {
        mChangeMode = cm;
        mDataPointLogger.stop();
    }

    mChangeMode |= cm;
    if (!mDataPointLogger.isActive())
        mDataPointLogger.start();
}

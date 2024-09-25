#include "PerfTestService.h"
#include "Config.h"
#include "DeviceInfo.h"
#include "LogHelper.h"
#include "DeviceControllerCPP.h"

#include <QCoreApplication>
#include <QRandomGenerator64>
#include <QTimer>

#define TEN_AM QTime::fromString("10:00:00")

PerfTestService* PerfTestService::mMe = nullptr;

PerfTestService* PerfTestService::me()
{
    if (!mMe)
        mMe = new PerfTestService(qApp);

    return mMe;
}

PerfTestService::PerfTestService(QObject *parent)
    : DevApiExecutor{parent}
{
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);
    mTimerGetTemp.setInterval(15 * 1000);
    connect(&mTimerGetTemp, &QTimer::timeout, this, &PerfTestService::collectReading);

    if (QDateTime::currentDateTime().secsTo(QDateTime(QDate::currentDate(), QTime::fromString("11:45:00"))) >= 0) {

        auto time = QTime::currentTime() > TEN_AM ? QTime::currentTime() : TEN_AM;
        scheduleNextCheck(QDateTime(QDate::currentDate(), time));
    }
    else {
        scheduleNextCheck(QDateTime(QDate::currentDate().addDays(1), TEN_AM));
    }
}

void PerfTestService::scheduleNextCheck(const QDateTime& scheduleDate)
{
    auto secondsToNextCheck = QDateTime::currentDateTime().msecsTo(scheduleDate);
    QTimer::singleShot(secondsToNextCheck, this, &PerfTestService::checkTestEligibility);
}

void PerfTestService::checkTestEligibility()
{
    if (Device->serialNumber().isEmpty()) {
        qWarning() << "Sn is not ready! can not get user-data!";
        return;
    }

    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &data) {
        isCoolingTest(data.value("cooling").toBool());
        isEligibleTotest(data.value("status").toBool());
    };

    callGetApi(API_SERVER_BASE_URL + QString("api/sync/perftest/schedule?sn=%0").arg(Device->serialNumber()), callback);
}

void PerfTestService::startTest()
{
    if (!mTimerGetTemp.isActive()) {
        mTimerGetTemp.start();
    }
}

void PerfTestService::collectReading()
{
    mReadings.append(QJsonValue(DeviceControllerCPP::instance()->getTemperature()));
}

void PerfTestService::cancelTest()
{
    mTimerGetTemp.stop();

    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &data) {
    };

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/cancel?sn=%0").arg(Device->serialNumber());
    callGetApi(url, callback);
}

void PerfTestService::sendReadingsToServer()
{
    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &data) {
        isCoolingTest(data.value("cooling").toBool());
        isEligibleTotest(data.value("status").toBool());
    };

    QJsonObject data;
    data["readings"] = mReadings;

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/result?sn=%0").arg(Device->serialNumber());
    callPostApi(url, QJsonDocument(data).toJson(), callback);
}

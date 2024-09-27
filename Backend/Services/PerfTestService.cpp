#include "PerfTestService.h"
#include "Config.h"
#include "DeviceInfo.h"
#include "LogHelper.h"
#include "AppSpecCPP.h"
#include "DeviceControllerCPP.h"

#include <QCoreApplication>
#include <QRandomGenerator64>
#include <QTimer>

PerfTestService* PerfTestService::mMe = nullptr;

PerfTestService* PerfTestService::me()
{
    if (!mMe) mMe = new PerfTestService(qApp);

    return mMe;
}

PerfTestService::PerfTestService(QObject *parent)
    : DevApiExecutor{parent}
{
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);
    mTimerGetTemp.setInterval(15 * 1000);
    connect(&mTimerGetTemp, &QTimer::timeout, this, &PerfTestService::collectReading);

    scheduleNextCheck(QTime::currentTime());
}

void PerfTestService::scheduleNextCheck(const QTime& checkTime)
{
    while (mReadings.count() > 0) mReadings.removeLast();

    state(TestState::Waiting);

    auto tenAm = QTime::fromString("10:00:00");
    QDateTime nextScheduleMark;
    QDateTime timeToCheckFrom = QDateTime(QDate::currentDate(), checkTime);

    if (timeToCheckFrom.secsTo(QDateTime(QDate::currentDate(), QTime::fromString("11:45:00"))) >= 0) {
        auto time = checkTime > tenAm ? checkTime : tenAm;
        nextScheduleMark = QDateTime(QDate::currentDate(), time);
    }
    else {
        nextScheduleMark = QDateTime(QDate::currentDate().addDays(1), tenAm);
    }

    auto msecsToNextCheck = timeToCheckFrom.msecsTo(nextScheduleMark);
    TRACE << "PERF-TEST: " << "Next Schedule time " << nextScheduleMark << (qreal)msecsToNextCheck / (1000* 60);
    QTimer::singleShot(30 * 1000, this, &PerfTestService::checkTestEligibility);
}

void PerfTestService::checkTestEligibility()
{
    if (Device->serialNumber().isEmpty()) {
        qWarning() << "PERF-TEST: " << "Sn is not ready! can not check perf-test-eligibility";
        scheduleNextCheck(QTime::currentTime().addSecs(10 * 60));
        return;
    }

    state(TestState::Checking);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        TRACE << "PERF-TEST: " << "CheckTestEligibility Response " << rawData;
        mode(data.value("cooling").toBool() ? AppSpecCPP::Cooling : AppSpecCPP::Heating);
        if (data.value("status").toBool()) {
            state(TestState::Eligible);
        }
        else {
            scheduleNextCheck(QTime::fromString("12:00:00"));
        }
    };

    callGetApi(API_SERVER_BASE_URL + QString("api/sync/perftest/schedule?sn=%0").arg(Device->serialNumber()), callback);
}

void PerfTestService::startTest()
{
    if (!mTimerGetTemp.isActive()) {
        mTimerGetTemp.start();
    }

    state(TestState::Warmup);
}

void PerfTestService::collectReading()
{
    mReadings.append(QJsonValue(DeviceControllerCPP::instance()->getTemperature()));
    if (mReadings.count() == 60) {
        mTimerGetTemp.stop();
        sendReadingsToServer();
    }
}

void PerfTestService::cancelTest()
{
    mTimerGetTemp.stop();
    state(TestState::Cancelling);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        TRACE << "PERF-TEST: " << "cancelTest Response " << rawData;
        scheduleNextCheck(QTime::fromString("12:00:00"));
    };

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/cancel?sn=%0").arg(Device->serialNumber());
    callGetApi(url, callback);
}

void PerfTestService::sendReadingsToServer()
{
    state(TestState::Sending);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        TRACE << "PERF-TEST: " << "sendReadingsToServer Response " << rawData;
        state(TestState::Complete);
    };

    QJsonObject data;
    data["readings"] = mReadings;

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/result?sn=%0").arg(Device->serialNumber());
    callPostApi(url, QJsonDocument(data).toJson(), callback);
}

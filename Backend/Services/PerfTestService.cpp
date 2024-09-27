#include "PerfTestService.h"
#include "Config.h"
#include "DeviceInfo.h"
#include "LogHelper.h"
#include "AppSpecCPP.h"
#include "DeviceControllerCPP.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QRandomGenerator64>
#include <QTimer>


Q_LOGGING_CATEGORY(PerfTestService)


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

    mTimerDelay.setInterval(1000);
    connect(&mTimerDelay, &QTimer::timeout, [this]() {
        if (startTimeLeft () > 0) {
            startTimeLeft(startTimeLeft()--);
        }
        else {
            mTimerDelay.stop();
        }
    });

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
    qCDebug(PerfTestService) << "Next Schedule time " << nextScheduleMark << (qreal)msecsToNextCheck / (1000* 60);
    QTimer::singleShot(30 * 1000, this, &PerfTestService::checkTestEligibility);
}

void PerfTestService::checkTestEligibility()
{
    if (Device->serialNumber().isEmpty()) {
        qCDebug(PerfTestService) << "Sn is not ready! can not check perf-test-eligibility";
        scheduleNextCheck(QTime::currentTime().addSecs(10 * 60));
        return;
    }

    state(TestState::Checking);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        qCDebug(PerfTestService)<< "CheckTestEligibility Response " << rawData;
        mode(data.value("cooling").toBool() ? AppSpecCPP::Cooling : AppSpecCPP::Heating);
        if (data.value("status").toBool()) {
            state(TestState::Eligible);
            setupWarmup();
        }
        else {
            scheduleNextCheck(QTime::fromString("12:00:00"));
        }
    };

    callGetApi(API_SERVER_BASE_URL + QString("api/sync/perftest/schedule?sn=%0").arg(Device->serialNumber()), callback);
}

void PerfTestService::setupWarmup()
{
    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::startSystemDelayCountdown, this, &PerfTestService::onCountdownStart);
    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::stopSystemDelayCountdown, this, &PerfTestService::onCountdownStop);

    if (DeviceControllerCPP::instance()->systemSetup()->systemMode == mode()) {
        startTest();
    }
    else {
        bool isHeatable = DeviceControllerCPP::instance()->systemSetup()->systemType != AppSpecCPP.CoolingOnly;
        bool isCoolable = DeviceControllerCPP::instance()->systemSetup()->systemType != AppSpecCPP.HeatingOnly;

        if ((mode() == AppSpecCPP::Heating && !isHeatable) || (mode() == AppSpecCPP::Cooling && !isCoolable)) {
            qCDebug(PerfTestService) << "Test requested mode not compatible. Requested-mode=" << mode()
                                     << ", Device-type=" << DeviceControllerCPP::instance()->systemSetup()->systemType;
            scheduleNextCheck(QTime::fromString("12:00:00"));
        }
        else {
            state(TestState::Warmup);
            DeviceControllerCPP::instance()->systemSetup()->setMode(mode());
        }
    }
}

void PerfTestService::onCountdownStart(AppSpecCPP::SystemMode mode, int delay)
{
    actualMode(mode);
    qCDebug(PerfTestService)<< "onCountdownStart Response " << mode << ", " << delay;
    startTimeLeft(delay/1000);
    mTimerDelay.start();
}

void PerfTestService::onCountdownStop()
{
    qCDebug(PerfTestService)<< "onCountdownStop Response " << mode << ", " << delay;
    mTimerDelay.stop();
    startTest();
}

void PerfTestService::startTest()
{
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::startSystemDelayCountdown, this, &PerfTestService::onCountdownStart);
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::stopSystemDelayCountdown, this, &PerfTestService::onCountdownStop);

    if (!mTimerGetTemp.isActive()) {
        mTimerGetTemp.start();
    }

    state(TestState::Running);
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
        qCDebug(PerfTestService)<< "cancelTest Response " << rawData;
        scheduleNextCheck(QTime::fromString("12:00:00"));
    };

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/cancel?sn=%0").arg(Device->serialNumber());
    callGetApi(url, callback);
}

void PerfTestService::sendReadingsToServer()
{
    state(TestState::Sending);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        qCDebug(PerfTestService)<< "sendReadingsToServer Response " << rawData;
        state(TestState::Complete);
    };

    QJsonObject data;
    data["readings"] = mReadings;

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/result?sn=%0").arg(Device->serialNumber());
    callPostApi(url, QJsonDocument(data).toJson(), callback);
}

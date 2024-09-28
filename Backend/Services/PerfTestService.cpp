#include "PerfTestService.h"
#include "Config.h"
#include "DeviceInfo.h"
#include "LogHelper.h"
#include "AppSpecCPP.h"
#include "DeviceControllerCPP.h"
#include "NetworkManager.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QRandomGenerator64>
#include <QTimer>


Q_LOGGING_CATEGORY(PerfTestServiceCat, "PerfTestServiceLog")


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
        auto timeLeft = startTimeLeft();
        if (timeLeft > 0) {
            startTimeLeft(timeLeft--);
        }
        else {
            startRunning();
        }
    });

    mTimerGetTemp.setInterval(1 * 1000); // TODO: 15 * 1000
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
    qCDebug(PerfTestServiceCat) << "Next Schedule time " << nextScheduleMark << (qreal)msecsToNextCheck / (1000* 60);
    QTimer::singleShot(30000, this, &PerfTestService::checkTestEligibility); // TODO: msecsToNextCheck
}

void PerfTestService::checkTestEligibility()
{
    if (Device->serialNumber().isEmpty()) {
        qCDebug(PerfTestServiceCat) << "Sn is not ready! can not check perf-test-eligibility";
        scheduleNextCheck(QTime::currentTime().addSecs(10 * 60));
        return;
    }

    state(TestState::Checking);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {        
        mode(data.value("cooling").toBool() ? AppSpecCPP::Cooling : AppSpecCPP::Heating);
        qCDebug(PerfTestServiceCat)<< "CheckTestEligibility Response " << rawData << mode();
        state(TestState::Eligible);
        setupWarmup();
        // if (data.value("status").toBool()) {
        //     state(TestState::Eligible);
        //     setupWarmup();
        // }
        // else {
        //     scheduleNextCheck(QTime::fromString("12:00:00"));
        // }
    };

    callGetApi(API_SERVER_BASE_URL + QString("api/sync/perftest/schedule?sn=%0").arg(Device->serialNumber()), callback);
}

void PerfTestService::setupWarmup()
{
    qCDebug(PerfTestServiceCat) << "setupWarmup";

    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::startSystemDelayCountdown, this, &PerfTestService::onCountdownStart);
    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::stopSystemDelayCountdown, this, &PerfTestService::onCountdownStop);
    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::tempSchemeStateChanged, this, &PerfTestService::onTempSchemeStateChanged);

    if (DeviceControllerCPP::instance()->systemSetup()->systemMode == mode()) {
        NetworkManager::instance()->isEnable(false);
        startRunning();
    }
    else {
        bool isHeatable = DeviceControllerCPP::instance()->systemSetup()->systemType != AppSpecCPP::CoolingOnly;
        bool isCoolable = DeviceControllerCPP::instance()->systemSetup()->systemType != AppSpecCPP::HeatingOnly;

        if ((mode() == AppSpecCPP::Heating && !isHeatable) || (mode() == AppSpecCPP::Cooling && !isCoolable)) {
            qCDebug(PerfTestServiceCat) << "Test requested mode is not compatible. Requested-mode=" << mode()
                                     << ", Device-type=" << DeviceControllerCPP::instance()->systemSetup()->systemType;
            scheduleNextCheck(QTime::fromString("12:00:00"));
        }
        else {
            state(TestState::Warmup);
            NetworkManager::instance()->isEnable(false);
            DeviceControllerCPP::instance()->doPerfTest(mode());
        }
    }
}

void PerfTestService::onCountdownStart(AppSpecCPP::SystemMode mode, int delay)
{
    actualMode(mode);    
    qCDebug(PerfTestServiceCat) << "onCountdownStart " << mode << ", " << delay;
    qCDebug(PerfTestServiceCat) << "State: " << state();
    startTimeLeft(delay/1000);
    mTimerDelay.start();
    mTimerGetTemp.stop();
    state(TestState::Warmup);
}

void PerfTestService::onCountdownStop()
{
    qCDebug(PerfTestServiceCat) << "onCountdownStop";
    qCDebug(PerfTestServiceCat) << "State: " << state();
    startRunning();
}

void PerfTestService::onTempSchemeStateChanged(bool started)
{
    if (started && startTimeLeft() <= 0) {
        startRunning();
    }
}

void PerfTestService::startRunning()
{
    qCDebug(PerfTestServiceCat) << "startRunning";
    qCDebug(PerfTestServiceCat) << "State: " << state();
    startTimeLeft(0);
    mTimerDelay.stop();
    mTimerGetTemp.start();
    state(TestState::Running);
}

void PerfTestService::collectReading()
{
    auto temperature = DeviceControllerCPP::instance()->getTemperature();
    qCDebug(PerfTestServiceCat) << "collectReading " << temperature;

    mReadings.append(QJsonValue(temperature));
    if (mReadings.count() == 60) {
        cleanupRunning();
        sendReadingsToServer();
    }
}

void PerfTestService::cleanupRunning()
{
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::startSystemDelayCountdown, this, &PerfTestService::onCountdownStart);
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::stopSystemDelayCountdown, this, &PerfTestService::onCountdownStop);
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::tempSchemeStateChanged, this, &PerfTestService::onTempSchemeStateChanged);
    NetworkManager::instance()->isEnable(true);
    mTimerGetTemp.stop();
    DeviceControllerCPP::instance()->revertPerfTest();
}

void PerfTestService::cancelTest()
{
    cleanupRunning();
    state(TestState::Cancelling);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        qCDebug(PerfTestServiceCat)<< "cancelTest Response " << rawData;
        scheduleNextCheck(QTime::fromString("12:00:00"));
    };

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/cancel?sn=%0").arg(Device->serialNumber());
    callGetApi(url, callback);
}

void PerfTestService::sendReadingsToServer()
{
    state(TestState::Sending);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        qCDebug(PerfTestServiceCat)<< "sendReadingsToServer Response " << rawData;
        state(TestState::Complete);
    };

    QJsonObject data;
    data["readings"] = mReadings;

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/result?sn=%0").arg(Device->serialNumber());
    callPostApi(url, QJsonDocument(data).toJson(), callback);
}

void PerfTestService::finishTest()
{
    if (state() == TestState::Complete) {
        qCDebug(PerfTestServiceCat)<< "finishTest, testing done, scheduling next check.";
        scheduleNextCheck(QTime::fromString("12:00:00"));
    }
}

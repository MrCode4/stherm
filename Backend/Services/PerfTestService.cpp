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


Q_LOGGING_CATEGORY(PerfTestLogCat, "PerfTestServiceLog")

namespace PerfTest {
const int OneSecInMS = 1000;
const QTime Noon12PM = QTime::fromString("12:00:00");
const QTime TenAM = QTime::fromString("10:00:00");
const int TestDuration = 2 * 60; //TODO: 15 * 60
const int DataPickInterval = 2; //TODO: 15
const int FinishDelay = 30; //TODO: 5 * 60
}

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

    mTimerDelay.setInterval(PerfTest::OneSecInMS);
    connect(&mTimerDelay, &QTimer::timeout, [this]() {
        auto timeLeft = startTimeLeft();        
        if (timeLeft > 0) {
            startTimeLeft(timeLeft - 1);
        }
        else {
            startRunning();
        }
    });

    mTimerGetTemp.setInterval(PerfTest::DataPickInterval * PerfTest::OneSecInMS);
    connect(&mTimerGetTemp, &QTimer::timeout, this, &PerfTestService::collectReading);

    mTimerFinish.setInterval(PerfTest::OneSecInMS);
    connect(&mTimerFinish, &QTimer::timeout, [this]() {
        auto timeLeft = finishTimeLeft();
        qCDebug(PerfTestLogCat)<<"finishTimeLeft " <<timeLeft;
        if (timeLeft > 0) {
            finishTimeLeft(timeLeft - 1);
        }
        else {
            mTimerFinish.stop();
            scheduleNextCheck(PerfTest::Noon12PM);
        }
    });

    scheduleNextCheck(QTime::currentTime());
}

void PerfTestService::scheduleNextCheck(const QTime& checkTime)
{
    while (mReadings.count() > 0) mReadings.removeLast();

    startTimeLeft(0);
    testTimeLeft(0);
    finishTimeLeft(0);
    state(TestState::Waiting);

    QDateTime nextScheduleMark;
    QDateTime timeToCheckFrom = QDateTime(QDate::currentDate(), checkTime);

    if (timeToCheckFrom.secsTo(QDateTime(QDate::currentDate(), PerfTest::Noon12PM.addSecs(-1 * PerfTest::TestDuration))) >= 0) {
        auto time = checkTime > PerfTest::TenAM ? checkTime : PerfTest::TenAM;
        nextScheduleMark = QDateTime(QDate::currentDate(), time);
    }
    else {
        nextScheduleMark = QDateTime(QDate::currentDate().addDays(1), PerfTest::TenAM);
    }

    auto msecsToNextCheck = timeToCheckFrom.msecsTo(nextScheduleMark);
    qCDebug(PerfTestLogCat) <<"Next Schedule time " <<nextScheduleMark <<msecsToNextCheck/(PerfTest::OneSecInMS) <<" ms";
    QTimer::singleShot(30000, this, &PerfTestService::checkTestEligibility); //TODO: msecsToNextCheck
}

void PerfTestService::checkTestEligibility()
{
    if (Device->serialNumber().isEmpty()) {
        qCDebug(PerfTestLogCat) <<"Sn is not ready! can not check perf-test-eligibility";
        scheduleNextCheck(QTime::currentTime().addSecs(PerfTest::TestDuration / 3));
        return;
    }

    state(TestState::Checking);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {        
        mode(data.value("cooling").toBool() ? AppSpecCPP::Cooling : AppSpecCPP::Heating);
        qCDebug(PerfTestLogCat)<<"CheckTestEligibility Response " <<rawData <<mode();
        state(TestState::Eligible);
        setupWarmup();
        // if (data.value("status").toBool()) {
        //     state(TestState::Eligible);
        //     setupWarmup();
        // }
        // else {
        //     scheduleNextCheck(PerfTest::Noon12PM);
        // }
    };

    callGetApi(API_SERVER_BASE_URL + QString("api/sync/perftest/schedule?sn=%0").arg(Device->serialNumber()), callback);
}

void PerfTestService::setupWarmup()
{
    qCDebug(PerfTestLogCat) <<"setupWarmup";

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
            qCDebug(PerfTestLogCat) <<"Test requested mode is not compatible. Requested-mode=" <<mode()
                                     <<", Device-type=" <<DeviceControllerCPP::instance()->systemSetup()->systemType;
            scheduleNextCheck(PerfTest::Noon12PM);
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
    qCDebug(PerfTestLogCat) <<"onCountdownStart " <<mode <<", " <<delay;
    qCDebug(PerfTestLogCat) <<"State: " <<state();
    startTimeLeft(delay/PerfTest::OneSecInMS);
    mTimerDelay.start();
    mTimerGetTemp.stop();
    state(TestState::Warmup);
}

void PerfTestService::onCountdownStop()
{
    qCDebug(PerfTestLogCat) <<"onCountdownStop";
    qCDebug(PerfTestLogCat) <<"State: " <<state();
    startRunning();
}

void PerfTestService::onTempSchemeStateChanged(bool started)
{
    qCDebug(PerfTestLogCat) <<"onTempSchemeStateChanged: " <<started;
    if (started) {
        testTimeLeft(PerfTest::TestDuration);
        startRunning();
    }
}

void PerfTestService::startRunning()
{
    qCDebug(PerfTestLogCat) <<"startRunning";
    qCDebug(PerfTestLogCat) <<"State: " <<state();
    startTimeLeft(0);
    mTimerDelay.stop();
    mTimerGetTemp.start();
    state(TestState::Running);
}

void PerfTestService::collectReading()
{
    auto temperature = DeviceControllerCPP::instance()->getTemperature();
    qCDebug(PerfTestLogCat) <<"collectReading " <<temperature;    
    testTimeLeft(testTimeLeft() - mTimerGetTemp.interval() / PerfTest::OneSecInMS);
    qCDebug(PerfTestLogCat) <<"testTimeLeft " <<testTimeLeft();
    mReadings.append(QJsonValue(temperature));
    if (testTimeLeft() <= 0) {
        cleanupRunning();
        sendReadingsToServer();
    }
}

void PerfTestService::cleanupRunning()
{
    qCDebug(PerfTestLogCat)<<"cleanupRunning";
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
    //state(TestState::Cancelling);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        qCDebug(PerfTestLogCat)<<"cancelTest Response " <<rawData;        
        scheduleNextCheck(PerfTest::Noon12PM);
    };

    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/cancel?sn=%0").arg(Device->serialNumber());
    callGetApi(url, callback);
}

void PerfTestService::sendReadingsToServer()
{
    qCDebug(PerfTestLogCat)<<"sendReadingsToServer";
    state(TestState::Sending);

    auto callback = [this](QNetworkReply *, const QByteArray &rawData, QJsonObject &data) {
        qCDebug(PerfTestLogCat)<<"sendReadingsToServer Response " <<rawData;
        qCDebug(PerfTestLogCat)<<"Testing result uploaded, waiting to finish by user or automatic in " << PerfTest::FinishDelay;        
        finishTimeLeft(PerfTest::FinishDelay);
        mTimerFinish.start();
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
        qCDebug(PerfTestLogCat)<<"finishTest, testing done, scheduling next check.";
        mTimerFinish.stop();
        scheduleNextCheck(PerfTest::Noon12PM);
    }
}

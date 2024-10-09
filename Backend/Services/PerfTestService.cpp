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
#include <QSettings>
#include <QTimer>


Q_LOGGING_CATEGORY(PerfTestLogCat, "PerfTestServiceLog")

namespace PerfTest {
const int OneSecInMS = 1000;
const int OneMinInMS = 60 * OneSecInMS;
const QTime Noon12PM = QTime::fromString("12:00:00");
const QTime TenAM = QTime::fromString("10:00:00");
const int TestDuration = 2 * 60; //TODO: 15 * 60
const int DataPickInterval = 2; //TODO: 15
const int FinishDelay = 10; //TODO: 5 * 60

const QString Heating("heating");
const QString Cooling("cooling");
const QString Finished("finished");
const QString Stopped("stopped");

const QString Key_TestID("perftest_id");
const QString Key_TestData("perftest_data");
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
    qCDebug(PerfTestLogCat) <<"PerfTestService";
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
        qCDebug(PerfTestLogCat) <<"finishTimeLeft " <<timeLeft;
        if (timeLeft > 0) {
            finishTimeLeft(timeLeft - 1);
        }
        else {
            finishTest();
        }
    });

    mTimerRetrySending.setInterval(5 * 60 * PerfTest::OneSecInMS);
    connect(&mTimerRetrySending, &QTimer::timeout, this, &PerfTestService::checkAndSendSavedResult);

    checkAndSendSavedResult();
    scheduleNextCheck(QTime::currentTime());
}

void PerfTestService::postponeTest(const QString &reason)
{
    if (state() > Checking) {
        qCDebug(PerfTestLogCat) <<"Perf-test can't be postponed since it's already running";
    }
    else {
        isPostponed(true);
        qCDebug(PerfTestLogCat) <<"Perf-test is postponed, reason: " <<reason;
    }
}

void PerfTestService::resumeTest()
{
    if (!isPostponed()) return;
    isPostponed(false);

    if (mWasEligibleBeforePostpone) {
        qCDebug(PerfTestLogCat) <<"Perf-test is elligible while resuming";
        mWasEligibleBeforePostpone = false;
        state(TestState::Eligible);
        setupWarmup();
    }
    else {
        qCDebug(PerfTestLogCat) <<"Perf-test was not elligible while resuming";
    }
}

void PerfTestService::scheduleNextCheck(const QTime& checkTime)
{
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
    QTimer::singleShot(60000, this, &PerfTestService::checkTestEligibility); //TODO: msecsToNextCheck
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
        auto perfId= data.value(PerfTest::Key_TestID).toInt();
        qCDebug(PerfTestLogCat) <<"CheckTestEligibility Response " <<perfId <<rawData ;

        QSettings settings;
        if (settings.contains(PerfTest::Key_TestID)) {
            if (settings.value(PerfTest::Key_TestID).toInt() == perfId) {
                qCDebug(PerfTestLogCat) <<"in checkTestEligibility, elligible test id is already finished testing and retrying uploading "<< perfId;
                scheduleNextCheck(PerfTest::Noon12PM);
                return;
            }
        }

        testId(perfId);
        mTimerRetrySending.stop();

        auto action = data.value("action").toString();
        if (action == PerfTest::Cooling) {
            mode(AppSpecCPP::Cooling);
        } else if (action == PerfTest::Heating) {
            mode(AppSpecCPP::Heating);
        } else {
            mode(AppSpecCPP::Off);
        }        

        if (testId() > 0 && mode() != AppSpecCPP::Off) {
            if (isPostponed()) {
                mWasEligibleBeforePostpone = true;
                // Check if blocking is not resumed by 12PM, ublock and schedule for next day
                QTimer::singleShot(qMax(PerfTest::OneSecInMS, QTime::currentTime().msecsTo(PerfTest::Noon12PM)), [this] () {
                    if (isPostponed()) {
                        qCDebug(PerfTestLogCat) <<"Perf-test was not resumed by 12 noon, so going for next-day";
                        isPostponed(false);
                        mWasEligibleBeforePostpone = false;
                        scheduleNextCheck(PerfTest::Noon12PM);
                    }
                });
            }
            else {
                state(TestState::Eligible);
                setupWarmup();
            }
        }
        else {
            scheduleNextCheck(PerfTest::Noon12PM);
        }
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
        startRunning();
    }
}

void PerfTestService::startRunning()
{
    if (state() == TestState::Running) return;

    testTimeLeft(PerfTest::TestDuration);
    qCDebug(PerfTestLogCat) <<"startRunning";
    qCDebug(PerfTestLogCat) <<"State: " <<state();
    startTimeLeft(0);
    mTimerDelay.stop();
    mTimerGetTemp.start();
    state(TestState::Running);
}

void PerfTestService::cleanupRunning()
{
    qCDebug(PerfTestLogCat) <<"cleanupRunning";
    while (mReadings.count() > 0) mReadings.removeLast();
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::startSystemDelayCountdown, this, &PerfTestService::onCountdownStart);
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::stopSystemDelayCountdown, this, &PerfTestService::onCountdownStop);
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::tempSchemeStateChanged, this, &PerfTestService::onTempSchemeStateChanged);
    NetworkManager::instance()->isEnable(true);
    mTimerGetTemp.stop();
    DeviceControllerCPP::instance()->revertPerfTest();
}

void PerfTestService::cancelTest()
{
    qCDebug(PerfTestLogCat) <<"Perf-test cancelled by user";
    QJsonObject data;
    data[PerfTest::Key_TestID] = testId();
    data["sn"] = Device->serialNumber();
    data["action"] = mode() == AppSpecCPP::Cooling ? PerfTest::Cooling : PerfTest::Heating;
    data["result"] = PerfTest::Stopped;
    data["time"] = QDateTime::currentDateTimeUtc().toString(DATETIME_FORMAT);
    auto jsonData = QJsonDocument(data).toJson();
    QSettings settings;
    settings.setValue(PerfTest::Key_TestData, jsonData);

    cleanupRunning();
    sendResultsToServer(Device->serialNumber(), jsonData);
    scheduleNextCheck(PerfTest::Noon12PM);
}

void PerfTestService::collectReading()
{
    auto temperature = DeviceControllerCPP::instance()->getTemperature();
    qCDebug(PerfTestLogCat) <<"collectReading " <<temperature;    
    testTimeLeft(testTimeLeft() - mTimerGetTemp.interval() / PerfTest::OneSecInMS);
    qCDebug(PerfTestLogCat) <<"testTimeLeft " <<testTimeLeft();
    QJsonObject item;
    item["timestamp"] = QDateTime::currentDateTimeUtc().toString(DATETIME_FORMAT);
    item["temperature"] = temperature;
    mReadings.append(item);

    if (testTimeLeft() <= 0) {
        qCDebug(PerfTestLogCat) <<"Perf-test getting readings completed";

        QJsonObject data;
        data[PerfTest::Key_TestID] = testId();
        data["sn"] = Device->serialNumber();
        data["action"] = mode() == AppSpecCPP::Cooling ? PerfTest::Cooling : PerfTest::Heating;
        data["result"] = PerfTest::Finished;
        data["time"] = QDateTime::currentDateTimeUtc().toString(DATETIME_FORMAT);
        data["data"] = mReadings;
        auto json = QJsonDocument(data).toJson();

        QSettings settings;
        settings.setValue(PerfTest::Key_TestID, testId());
        settings.setValue(PerfTest::Key_TestData, json);

        cleanupRunning();
        sendResultsToServer(Device->serialNumber(), json);
        finishTimeLeft(PerfTest::FinishDelay);
        mTimerFinish.start();
        state(TestState::Complete);
    }
}

void PerfTestService::checkAndSendSavedResult()
{
    QSettings settings;
    if (settings.contains(PerfTest::Key_TestData)) {
        auto data = settings.value(PerfTest::Key_TestData).toByteArray();
        qCDebug(PerfTestLogCat) <<"Perf-test saved result found";
        auto dataObj = QJsonDocument::fromJson(data).object();
        auto sn = dataObj.value("sn").toString();
        auto testTime = QDateTime::fromString(dataObj.value("time").toString(), DATETIME_FORMAT);
        auto retryDays = dataObj.contains("data") ? 30 : 1;
        if (testTime.isValid() && testTime.daysTo(QDateTime::currentDateTimeUtc()) <= retryDays) {
            qCDebug(PerfTestLogCat) <<"Perf-test saved result sending retrying";
            sendResultsToServer(sn, data);
        }
        else {
            qCDebug(PerfTestLogCat) <<"Perf-test saved result expired, cleaning up";
            settings.remove(PerfTest::Key_TestID);
            settings.remove(PerfTest::Key_TestData);
        }
    }
}

void PerfTestService::sendResultsToServer(const QString& sn, const QByteArray& data)
{
    qCDebug(PerfTestLogCat) <<"sendResultsToServer";

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        qCDebug(PerfTestLogCat) <<"sendResultsToServer Response " <<rawData;

        if (reply->error() == QNetworkReply::NoError) {
            mTimerRetrySending.stop();
            qCDebug(PerfTestLogCat) <<"Perf-test result uploaded successfully";

            QSettings settings;
            settings.remove(PerfTest::Key_TestID);
            settings.remove(PerfTest::Key_TestData);
        }
        else {
            qCDebug(PerfTestLogCat) <<"Perf-test result uploading failed, retry scheduled";
            if (!mTimerRetrySending.isActive()) {
                mTimerRetrySending.start();
            }
        }
    };    

    qCDebug(PerfTestLogCat) <<"sendResultsToServer Request " <<data;
    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/result?sn=%0").arg(sn);
    callPostApi(url, data, callback);
}

void PerfTestService::finishTest()
{
    if (state() == TestState::Complete) {
        qCDebug(PerfTestLogCat) <<"finishTest, testing done, scheduling next check.";
        mTimerFinish.stop();
        scheduleNextCheck(PerfTest::Noon12PM);
    }
}

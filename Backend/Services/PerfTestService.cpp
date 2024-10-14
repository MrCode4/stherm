#include "PerfTestService.h"
#include "Config.h"
#include "DeviceInfo.h"
#include "LogHelper.h"
#include "AppSpecCPP.h"
#include "DeviceControllerCPP.h"
#include "NetworkManager.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>


Q_LOGGING_CATEGORY(PerfTestLogCat, "PerfTestServiceLog")

namespace PerfTest {
const int OneSecInMS = 1000;
const int OneMinInMS = 60 * OneSecInMS;
const QTime Noon12PM = QTime::fromString("12:00:00");
const QTime TenAM = QTime::fromString("10:00:00");
// In seconds
const int TestDuration = 15 * 60;
const int DataPickInterval = 15;
const int FinishDelay = 5 * 60;

const QString Mode_Heating("heating");
const QString Mode_Cooling("cooling");

const QString Act_Running("running");
const QString Act_Stopped("stopped");
const QString Act_Finished("finished");

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
    TRACE_CAT(PerfTestLogCat) <<"PerfTestService Initialize";
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);

    connect(&mTimerScheduleWatcher, &QTimer::timeout, this, &PerfTestService::checkTestEligibility);

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

    connect(&mTimerPostponeWatcher, &QTimer::timeout, [this]() {
        mTimerPostponeWatcher.stop();
        if (!isPostponed()) return;
        isPostponed(false);
        mWasEligibleBeforePostpone = false;
        TRACE_CAT(PerfTestLogCat) <<"Perf-test was not resumed by 12 noon, so going for next-day";
        scheduleNextCheck(PerfTest::Noon12PM);
    });

    mTimerGetTemp.setInterval(PerfTest::DataPickInterval * PerfTest::OneSecInMS);
    connect(&mTimerGetTemp, &QTimer::timeout, this, &PerfTestService::collectReading);

    mTimerFinish.setInterval(PerfTest::OneSecInMS);
    connect(&mTimerFinish, &QTimer::timeout, [this]() {
        auto timeLeft = finishTimeLeft();
        TRACE_CAT(PerfTestLogCat) <<"finishTimeLeft " <<timeLeft;
        if (timeLeft > 0) {
            finishTimeLeft(timeLeft - 1);
        }
        else {
            finishTest();
        }
    });

    mTimerRetrySending.setInterval(5 * PerfTest::OneMinInMS);
    connect(&mTimerRetrySending, &QTimer::timeout, [this]() {checkAndSendSavedResult();});

    checkAndSendSavedResult(true);
    scheduleNextCheck(QTime::currentTime());
}

void PerfTestService::postponeTest(const QString &reason)
{
    if (state() >= Warmup) {
        TRACE_CAT(PerfTestLogCat) <<"Perf-test can't be postponed since it's already running";
    }
    else {
        isPostponed(true);
        TRACE_CAT_CHECK(PerfTestLogCat, state() != Idle) <<"Perf-test is postponed, reason: " <<reason;
    }
}

void PerfTestService::resumeTest()
{
    if (!isPostponed()) return;
    mTimerPostponeWatcher.stop();
    isPostponed(false);

    if (mWasEligibleBeforePostpone) {
        TRACE_CAT(PerfTestLogCat) <<"Perf-test is eligible while resuming";
        mWasEligibleBeforePostpone = false;
        prepareStartRunning();
    }
    else {
        TRACE_CAT_CHECK(PerfTestLogCat, state() != Idle) <<"Perf-test was not eligible while resuming";
    }
}

void PerfTestService::scheduleNextCheck(const QTime& checkTime)
{
    startTimeLeft(0);
    testTimeLeft(0);
    finishTimeLeft(0);
    state(TestState::Idle);

    QDateTime nextScheduleMark;
    QDateTime timeToCheckFrom = QDateTime(QDate::currentDate(), checkTime);

    auto randomDelaySecs = QRandomGenerator::global()->bounded(0, PerfTest::TestDuration);

    if (timeToCheckFrom.secsTo(QDateTime(QDate::currentDate(), PerfTest::Noon12PM.addSecs(-1 * PerfTest::TestDuration))) >= 0) {
        auto time = checkTime > PerfTest::TenAM ? checkTime : PerfTest::TenAM.addSecs(randomDelaySecs);
        nextScheduleMark = QDateTime(QDate::currentDate(), time);
    }
    else {
        nextScheduleMark = QDateTime(QDate::currentDate().addDays(1), PerfTest::TenAM.addSecs(randomDelaySecs));
    }

    auto msecsToNextCheck = timeToCheckFrom.msecsTo(nextScheduleMark);
    TRACE_CAT(PerfTestLogCat) <<"Next Schedule time " <<nextScheduleMark <<msecsToNextCheck/(PerfTest::OneSecInMS) <<"seconds";
    mTimerScheduleWatcher.setInterval(msecsToNextCheck);
    mTimerScheduleWatcher.start();
}

void PerfTestService::checkTestEligibility()
{    
    mTimerScheduleWatcher.stop();

    if (Device->serialNumber().isEmpty()) {
        TRACE_CAT(PerfTestLogCat) <<"Sn is not ready! can not check perf-test-eligibility, will try next day";
        scheduleNextCheck(PerfTest::Noon12PM);
        return;
    }

    auto callback = [this](QNetworkReply* reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() != QNetworkReply::NoError) {
            TRACE_CAT(PerfTestLogCat) <<"CheckTestEligibility API failed, going to retry in 15 minutes";
            emit eligibilityChecked("Failed due to network issues, will retry in 15 mininutes");
            scheduleNextCheck(QTime::currentTime().addSecs(PerfTest::TestDuration));
            return;
        }

        auto perfId= data.value(PerfTest::Key_TestID).toInt();
        TRACE_CAT(PerfTestLogCat) <<"CheckTestEligibility response" <<perfId <<rawData ;

        QSettings settings;
        if (settings.contains(PerfTest::Key_TestID)) {
            if (settings.value(PerfTest::Key_TestID).toInt() == perfId) {
                TRACE_CAT(PerfTestLogCat) <<"in checkTestEligibility, eligible test id is already finished testing and retrying uploading "<< perfId;
                emit eligibilityChecked(QString("Eligible, but the test with id %1 have already performed once and now retrying sending the results back.").arg((perfId)));
                scheduleNextCheck(PerfTest::Noon12PM);
                return;
            }
        }        

        auto action = data.value("action").toString();
        AppSpecCPP::SystemMode testMode = AppSpecCPP::Off;
        if (action == PerfTest::Mode_Cooling) {
            testMode = AppSpecCPP::Cooling;
        }
        else if (action == PerfTest::Mode_Heating) {
            testMode = AppSpecCPP::Heating;
        }

        bool isHeatable = DeviceControllerCPP::instance()->systemSetup()->systemType != AppSpecCPP::CoolingOnly;
        bool isCoolable = DeviceControllerCPP::instance()->systemSetup()->systemType != AppSpecCPP::HeatingOnly;
        if ((testMode == AppSpecCPP::Heating && !isHeatable) || (testMode == AppSpecCPP::Cooling && !isCoolable)) {
            TRACE_CAT(PerfTestLogCat) <<"Perf-test is eligible but requested mode is not compatible. Requested-mode=" <<testMode
                                      <<", Device-type=" <<DeviceControllerCPP::instance()->systemSetup()->systemType;
            emit eligibilityChecked("Perf-test is eligible but requested mode is not compatible with system type.");
            scheduleNextCheck(PerfTest::Noon12PM);
            return;
        }

        if (perfId <= 0 || testMode == AppSpecCPP::Off) {
            emit eligibilityChecked(QString("Eligible, however, the test with id %1 have already performed once and now retrying sending the results back.").arg((perfId)));
            scheduleNextCheck(PerfTest::Noon12PM);
            return;
        }

        mTimerRetrySending.stop();
        testId(perfId);
        mode(testMode);
        state(Eligible);        
        emit eligibilityChecked("");

        if (isPostponed()) {
            TRACE_CAT(PerfTestLogCat) << "Eligible to perf-test but UI is busy, so postponing now until UI resumes";
            mWasEligibleBeforePostpone = true;
            // Check if blocking is not resumed by 12PM, ublock and schedule for next day
            mTimerPostponeWatcher.setInterval(12 * 60 * PerfTest::OneMinInMS);
            mTimerPostponeWatcher.start();
        }
        else {
            TRACE_CAT(PerfTestLogCat) << "Eligible to perf-test, so going for it now.";
            prepareStartRunning();
        }
    };

    state(Checking);
    callGetApi(API_SERVER_BASE_URL + QString("api/sync/perftest/schedule?sn=%0").arg(Device->serialNumber()), callback);
}

void PerfTestService::prepareStartRunning()
{
    TRACE_CAT(PerfTestLogCat) <<"prepareStartRunning";
    prepareAndSendApiResult(PerfTest::Act_Running);
}

void PerfTestService::checkWarmupOrRun()
{
    TRACE_CAT(PerfTestLogCat) <<"checkWarmupOrRun";

    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::startSystemDelayCountdown, this, &PerfTestService::onCountdownStart);
    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::stopSystemDelayCountdown, this, &PerfTestService::onCountdownStop);
    connect(DeviceControllerCPP::instance(), &DeviceControllerCPP::tempSchemeStateChanged, this, &PerfTestService::onTempSchemeStateChanged);
    NetworkManager::instance()->isEnable(false);

    if (DeviceControllerCPP::instance()->systemSetup()->systemMode == mode()) {
        TRACE_CAT(PerfTestLogCat) <<"Perf-test mode is same as current mode, so going for straight check " <<mode();
        startRunning();
    }
    else {
        TRACE_CAT(PerfTestLogCat) <<"Perf-test mode is different than current mode, so going mode change "
                                  <<DeviceControllerCPP::instance()->systemSetup()->systemMode <<mode();
        state(TestState::Warmup);
        DeviceControllerCPP::instance()->doPerfTest(mode());
    }
}

void PerfTestService::onCountdownStart(AppSpecCPP::SystemMode mode, int delay)
{
    TRACE_CAT(PerfTestLogCat) <<"onCountdownStart " <<mode <<", " <<delay <<state();
    startTimeLeft(delay/PerfTest::OneSecInMS);
    mTimerDelay.start();
    mTimerGetTemp.stop();
    state(TestState::Warmup);
}

void PerfTestService::onCountdownStop()
{
    TRACE_CAT(PerfTestLogCat) <<"onCountdownStop"<< state();
    startRunning();
}

void PerfTestService::onTempSchemeStateChanged(bool started)
{
    TRACE_CAT(PerfTestLogCat) <<"onTempSchemeStateChanged: " <<started;
    if (started) {        
        startRunning();
    }
}

void PerfTestService::startRunning()
{
    if (state() == TestState::Running) return;

    testTimeLeft(PerfTest::TestDuration);
    TRACE_CAT(PerfTestLogCat) <<"startRunning";
    TRACE_CAT(PerfTestLogCat) <<"State: " <<state();
    startTimeLeft(0);
    mTimerDelay.stop();
    mTimerGetTemp.start();
    state(TestState::Running);
}

void PerfTestService::cleanupRunning()
{
    TRACE_CAT(PerfTestLogCat) <<"cleanupRunning";    
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::startSystemDelayCountdown, this, &PerfTestService::onCountdownStart);
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::stopSystemDelayCountdown, this, &PerfTestService::onCountdownStop);
    disconnect(DeviceControllerCPP::instance(), &DeviceControllerCPP::tempSchemeStateChanged, this, &PerfTestService::onTempSchemeStateChanged);

    mTimerGetTemp.stop();
    NetworkManager::instance()->isEnable(true);    
    DeviceControllerCPP::instance()->revertPerfTest();
}

void PerfTestService::cancelTest()
{
    TRACE_CAT(PerfTestLogCat) <<"Perf-test cancelled by user";
    cleanupRunning();
    prepareAndSendApiResult(PerfTest::Act_Stopped);
    while (mReadings.count() > 0) mReadings.removeLast();
    scheduleNextCheck(PerfTest::Noon12PM);
}

void PerfTestService::collectReading()
{
    auto temperature = (DeviceControllerCPP::instance()->getTemperature() - 32) / 1.8;;
    TRACE_CAT(PerfTestLogCat) <<"collectReading " <<temperature;
    testTimeLeft(testTimeLeft() - mTimerGetTemp.interval() / PerfTest::OneSecInMS);
    TRACE_CAT(PerfTestLogCat) <<"testTimeLeft " <<testTimeLeft();    
    QJsonObject item;
    item["timestamp"] = QDateTime::currentDateTimeUtc().toString(DATETIME_FORMAT);
    item["temperature"] = temperature;
    mReadings.append(item);

    if (testTimeLeft() <= 0) {
        TRACE_CAT(PerfTestLogCat) <<"Perf-test getting readings completed";
        cleanupRunning();
        prepareAndSendApiResult(PerfTest::Act_Finished);
        while (mReadings.count() > 0) mReadings.removeLast();
        finishTimeLeft(PerfTest::FinishDelay);
        mTimerFinish.start();
        state(TestState::Complete);
    }
}

void PerfTestService::prepareAndSendApiResult(const QString &act)
{
    QJsonObject data;
    data[PerfTest::Key_TestID] = testId();
    data["sn"] = Device->serialNumber();
    data["action"] = mode() == AppSpecCPP::Cooling ? PerfTest::Mode_Cooling : PerfTest::Mode_Heating;
    data["result"] = act;
    data["time"] = QDateTime::currentDateTimeUtc().toString(DATETIME_FORMAT);
    if (act == PerfTest::Act_Finished) {
        data["data"] = mReadings;
    }

    auto json = QJsonDocument(data).toJson();    

    if (act == PerfTest::Act_Finished) {
        // Save to retry after reboot in case sending fails
        QSettings settings;
        settings.setValue(PerfTest::Key_TestData, json);
        settings.setValue(PerfTest::Key_TestID, testId());
    }

    sendResultsToServer(Device->serialNumber(), json);
}

void PerfTestService::checkAndSendSavedResult(bool checkTestId)
{
    QSettings settings;
    if ((!checkTestId || settings.contains(PerfTest::Key_TestID)) && settings.contains(PerfTest::Key_TestData)) {
        auto data = settings.value(PerfTest::Key_TestData).toByteArray();
        TRACE_CAT(PerfTestLogCat) <<"Perf-test saved result found";
        auto dataObj = QJsonDocument::fromJson(data).object();
        auto sn = dataObj.value("sn").toString();
        auto testTime = QDateTime::fromString(dataObj.value("time").toString(), DATETIME_FORMAT);
        auto retryDays = dataObj.contains("data") ? 30 : 1;
        if (testTime.isValid() && testTime.daysTo(QDateTime::currentDateTimeUtc()) <= retryDays) {
            TRACE_CAT(PerfTestLogCat) <<"Perf-test saved result sending retrying";
            sendResultsToServer(sn, data);
        }
        else {
            TRACE_CAT(PerfTestLogCat) <<"Perf-test saved result expired, cleaning up";
            settings.remove(PerfTest::Key_TestID);
            settings.remove(PerfTest::Key_TestData);
        }
    }
}

void PerfTestService::sendResultsToServer(const QString& sn, const QByteArray& resultData)
{
    auto resultType = QJsonDocument::fromJson(resultData).object().value("result").toString();

    auto callback = [this, resultType](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        TRACE_CAT(PerfTestLogCat) <<"sendResultsToServer response of type:" <<resultType <<", data:" <<rawData;

        if (reply->error() == QNetworkReply::NoError) {
            mTimerRetrySending.stop();
            TRACE_CAT(PerfTestLogCat) <<"Perf-test result API called successfully";

            QSettings settings;
            settings.remove(PerfTest::Key_TestID);
            settings.remove(PerfTest::Key_TestData);            
        }
        else {            
            if (resultType == PerfTest::Act_Finished && !mTimerRetrySending.isActive()) {
                TRACE_CAT(PerfTestLogCat) <<"Perf-test result uploading failed, retry scheduled";
                mTimerRetrySending.start();
            }
        }

        handleResultUpload(reply, resultType, data);
    };

    TRACE_CAT(PerfTestLogCat) <<"sendResultsToServer request type:" <<resultType <<", data:" <<resultData;
    auto url = API_SERVER_BASE_URL + QString("api/sync/perftest/result?sn=%0").arg(sn);
    callPostApi(url, resultData, callback);
}

void PerfTestService::handleResultUpload(QNetworkReply* reply, const QString& resultType, const QJsonObject& data)
{
    if (resultType == PerfTest::Act_Running) {
        TRACE_CAT(PerfTestLogCat) <<"Sent running status update to server";
        checkWarmupOrRun();
    }
}

void PerfTestService::finishTest()
{
    if (state() == TestState::Complete) {
        TRACE_CAT(PerfTestLogCat) <<"finishTest, testing done, scheduling next check.";
        mTimerFinish.stop();
        scheduleNextCheck(PerfTest::Noon12PM);
    }
}

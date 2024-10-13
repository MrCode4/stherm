#pragma once

#include "DevApiExecutor.h"
#include "Property.h"
#include "AppSpecCPP.h"

#include <QQmlEngine>
#include <QDateTime>
#include <QTimer>
#include <QVariantList>

class PerfTestService : public DevApiExecutor
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    PROPERTY_PRI_DEF_VAL(int, testId, 1)
    PROPERTY_PRI_DEF_VAL(int, state, 0)
    PROPERTY_PRI_DEF_VAL(AppSpecCPP::SystemMode, mode, AppSpecCPP::Cooling)
    PROPERTY_PRI_DEF_VAL(int, startTimeLeft, 0) // in seconds
    PROPERTY_PRI_DEF_VAL(int, testTimeLeft, 0) // in seconds
    PROPERTY_PRI_DEF_VAL(int, finishTimeLeft, 0) // in seconds
    PROPERTY_PRI_DEF_VAL(bool, isPostponed, false)

private:
    explicit PerfTestService(QObject* parent = nullptr);

public:
    static PerfTestService* me();
    static PerfTestService* create(QQmlEngine*, QJSEngine*) {return me();}

    enum TestState {
        Idle = 0,
        Checking,
        Eligible,
        Warmup,
        Running,
        Complete
    };
    Q_ENUM(TestState)

signals:

public slots:
    Q_INVOKABLE void checkTestEligibility();
    Q_INVOKABLE void postponeTest(const QString& reason);
    Q_INVOKABLE void resumeTest();
    Q_INVOKABLE void cancelTest();
    Q_INVOKABLE void finishTest();

private slots:
    void onCountdownStart(AppSpecCPP::SystemMode mode, int delay);
    void onCountdownStop();
    void onTempSchemeStateChanged(bool started);
    void collectReading();
    void checkAndSendSavedResult(bool checkTestId = false);

private:
    void scheduleNextCheck(const QTime& checkTime);
    void prepareStartRunning();
    void checkWarmupOrRun();
    void prepareAndSendApiResult(const QString& act);
    void startRunning();
    void cleanupRunning();    
    void sendResultsToServer(const QString& sn, const QByteArray& data);
    void handleResultUpload(QNetworkReply* reply, const QString& resultType, const QJsonObject& data);

private:
    static PerfTestService* mMe;

    bool mWasEligibleBeforePostpone = false;
    QTimer mTimerScheduleWatcher;
    QTimer mTimerPostponeWatcher;
    QTimer mTimerDelay;    
    QTimer mTimerGetTemp;
    QTimer mTimerFinish;
    QTimer mTimerRetrySending;
    QJsonArray mReadings;
};

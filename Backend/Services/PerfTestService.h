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

    PROPERTY_PRI_DEF_VAL(int, state, 0)
    PROPERTY_PRI_DEF_VAL(AppSpecCPP::SystemMode, mode, AppSpecCPP::Cooling)
    PROPERTY_PRI_DEF_VAL(AppSpecCPP::SystemMode, actualMode, AppSpecCPP::Cooling)
    PROPERTY_PRI_DEF_VAL(int, startTimeLeft, 0)
    PROPERTY_PRI_DEF_VAL(int, testTimeLeft, 0)

private:
    explicit PerfTestService(QObject* parent = nullptr);

public:
    static PerfTestService* me();
    static PerfTestService* create(QQmlEngine*, QJSEngine*) {return me();}

    enum TestState {
        Waiting = 0,
        Checking,
        Eligible,
        Warmup,
        Running,
        Sending,
        Complete,
        Cancelling
    };
    Q_ENUM(TestState)

signals:

public slots:
    Q_INVOKABLE void startTest();
    Q_INVOKABLE void stopTest();

private slots:
    void checkTestEligibility();
    void onCountdownStart(AppSpecCPP::SystemMode mode, int delay);
    void onCountdownStop();
    void onTemperatureSchemeStateChanged(bool started);
    void collectReading();

private:
    void scheduleNextCheck(const QTime& checkTime);
    void setupWarmup();
    void sendReadingsToServer();

private:
    static PerfTestService* mMe;

    QTimer mTimerDelay;
    QTimer mTimerGetTemp;
    QJsonArray mReadings;
};

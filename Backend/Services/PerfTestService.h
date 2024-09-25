#pragma once

#include "DevApiExecutor.h"
#include "Property.h"

#include <QQmlEngine>
#include <QDateTime>
#include <QTimer>
#include <QVariantList>

class PerfTestService : public DevApiExecutor
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    PROPERTY_PRI_DEF_VAL(bool, isEligibleTotest, false)
    PROPERTY_PRI_DEF_VAL(bool, isCoolingTest, false)
    PROPERTY_PRI_DEF_VAL(int, startTimeLeft, 0)
    PROPERTY_PRI_DEF_VAL(int, testTimeLeft, 0)

private:
    explicit PerfTestService(QObject* parent = nullptr);

public:
    static PerfTestService* me();
    static PerfTestService* create(QQmlEngine*, QJSEngine*) {return me();}

signals:

public slots:
    Q_INVOKABLE void startTest();
    Q_INVOKABLE void cancelTest();

private slots:
    void checkTestEligibility();
    void collectReading();

private:
    void scheduleNextCheck(const QDateTime& scheduleDate);
    void sendReadingsToServer();

private:
    static PerfTestService* mMe;

    QTimer mTimerGetTemp;
    QJsonArray mReadings;
};

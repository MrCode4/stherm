#pragma once

#include "DevApiExecutor.h"
#include "Property.h"

#include <QQmlEngine>
#include <QDateTime>

class PerfTestService : public DevApiExecutor
{
    Q_OBJECT
    QML_ELEMENT

    PROPERTY_PRI_DEF_VAL(bool, fetchingPerfTestSchedule, false)
    PROPERTY_PRI_DEF_VAL(int, startTimeLeft, 0)
    PROPERTY_PRI_DEF_VAL(int, testTimeLeft, 0)

public:
    explicit PerfTestService(QObject *parent = nullptr);

signals:

private slots:
    void checkTestEligibility();

private:
    void scheduleNextCheck(const QDateTime& scheduleDate);
};

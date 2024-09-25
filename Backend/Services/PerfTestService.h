#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QDateTime>

#include "Property.h"

class PerfTestService : public QObject
{
    Q_OBJECT
    QML_ELEMENT

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

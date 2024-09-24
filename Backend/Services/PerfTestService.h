#pragma once

#include <QQmlEngine>

#include "RestApiExecutor.h"
#include "Property.h"

class PerfTestService : public RestApiExecutor
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    PROPERTY_PRI_DEF_VAL(int, startTimeLeft, 0)
    PROPERTY_PRI_DEF_VAL(int, testTimeLeft, 0)

public:
    explicit PerfTestService(QObject *parent = nullptr);

signals:

private:
    void scheduleNextCheck(const QDate& scheduleDate);
};

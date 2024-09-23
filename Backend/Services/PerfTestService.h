#pragma once

#include <QQmlEngine>

#include "RestApiExecutor.h"
#include "Property.h"

class PerfTestService : public RestApiExecutor
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit PerfTestService(QObject *parent = nullptr);

signals:
};

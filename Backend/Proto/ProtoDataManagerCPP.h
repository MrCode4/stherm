#pragma once

#include <QObject>
#include <QQmlEngine>

#include "DevApiExecutor.h"

class ProtoDataManagerCPP : public DevApiExecutor
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit ProtoDataManagerCPP(QObject *parent = nullptr);

    Q_INVOKABLE void updateData();
signals:
};


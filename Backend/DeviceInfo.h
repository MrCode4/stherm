#pragma once

#include <QObject>
#include <string>
#include <QQmlEngine>

#include "Property.h"

/*! ***********************************************************************************************
 * Basic Device Information Store
 * ************************************************************************************************/

#define Device (DeviceInfo::me())

class DeviceInfo : public QObject
{
    Q_OBJECT
    DeviceInfo(QObject* parent = nullptr);

    PROPERTY_PUB(std::string, uid)
    PROPERTY_PRI(QString, serialNumber)
    PROPERTY_PRI_DEF_VAL(bool, hasClient, false)

public:
    static DeviceInfo* me();

public:
    bool updateSerialNumber(const QString& sn, bool clientSet);
    void reset();

private:
    static DeviceInfo* mMe;
};


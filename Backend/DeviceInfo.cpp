#include "DeviceInfo.h"
#include "LogHelper.h"

#include <QCoreApplication>
#include <QSettings>
#include <QDebug>

DeviceInfo* DeviceInfo::mMe = nullptr;

DeviceInfo* DeviceInfo::me()
{
    if (!mMe)
        mMe = new DeviceInfo(qApp);

    return mMe;
}

DeviceInfo::DeviceInfo(QObject *parent)
    : QObject(parent)
{
    QSettings setting;
    hasClient(setting.value("NUVE/SerialNumberClient").toBool());
    serialNumber(setting.value("NUVE/SerialNumber").toString());
}

bool DeviceInfo::updateSerialNumber(const QString& sn, bool clientSet)
{
    if (sn.isEmpty() || sn == serialNumber()){
        TRACE << "serial number not set:" << sn << ", current is :" << serialNumber();
        return false;
    }

    hasClient(clientSet);
    serialNumber(sn);

    QSettings setting;
    setting.setValue("NUVE/SerialNumberClient", hasClient());
    setting.setValue("NUVE/SerialNumber", serialNumber());

    return true;
}

void DeviceInfo::reset()
{
    updateSerialNumber("", false);
}

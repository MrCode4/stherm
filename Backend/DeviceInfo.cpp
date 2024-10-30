#include "DeviceInfo.h"
#include "LogHelper.h"

#include <QCoreApplication>
#include <QSettings>
#include <QDebug>

namespace NUVE {
const QString Key_SN("NUVE/SerialNumber");
const QString Key_Hasclient("NUVE/SerialNumberClient");
}

DeviceInfo* DeviceInfo::mMe = nullptr;

DeviceInfo* DeviceInfo::me()
{
    if (!mMe) {
        mMe = new DeviceInfo(qApp);
    }

    return mMe;
}

DeviceInfo::DeviceInfo(QObject *parent)
    : QObject(parent)
{
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);

#if !defined(FAKE_UID_MODE_ON) && !defined(INITIAL_SETUP_MODE_ON)
    QSettings setting;
    hasClient(setting.value(NUVE::Key_Hasclient).toBool());
    serialNumber(setting.value(NUVE::Key_SN).toString());
#endif
}

bool DeviceInfo::updateSerialNumber(const QString& sn, bool clientSet)
{
    if (sn.isEmpty() || sn == serialNumber()){
        TRACE << "serial number not set:" << sn << ", current is :" << serialNumber();
        return false;
    }

    hasClient(clientSet);
    serialNumber(sn);

#if !defined(FAKE_UID_MODE_ON) && !defined(INITIAL_SETUP_MODE_ON)
    QSettings setting;
    setting.setValue(NUVE::Key_Hasclient, hasClient());
    setting.setValue(NUVE::Key_SN, serialNumber());
#endif
    return true;
}

void DeviceInfo::reset()
{
    updateSerialNumber("", false);
}

#include "NetworkInterface.h"

#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>

NetworkInterface::NetworkInterface(QObject *parent)
    : QObject{parent}
    , mWifiReadProc { nullptr }
{
}

NetworkInterface::WifiInfoList NetworkInterface::wifis()
{
    return QQmlListProperty<WifiInfo>(this, nullptr,
        &NetworkInterface::networkCount,
        &NetworkInterface::networkAt);
}

void NetworkInterface::refereshWifis(bool forced)
{
}

WifiInfo* NetworkInterface::networkAt(WifiInfoList* list, qsizetype index)
{
    if (NetworkInterface* ni = qobject_cast<NetworkInterface*>(list->object)) {
        if (index >= 0 && index < ni->mWifiInfos.count()) {
            return ni->mWifiInfos.at(index);
        }
    }

    return nullptr;
}

qsizetype NetworkInterface::networkCount(WifiInfoList* list)
{
    if (NetworkInterface* ni = qobject_cast<NetworkInterface*>(list->object)) {
        return ni->mWifiInfos.count();
    }

    return 0;
}

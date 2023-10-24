#include "NetworkInterface.h"
#include "NmcliInterface.h"

#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>

NetworkInterface::NetworkInterface(QObject *parent)
    : QObject{parent}
    , mNmcliInterface { new NmcliInterface(this) }
    , mWifiReadProc { nullptr }
    , mConnectedWifiInfo { nullptr }
    , mRequestedToConnectedWifi { nullptr }
{
    connect(mNmcliInterface, &NmcliInterface::wifiListRefereshed, this, &NetworkInterface::onWifiListRefreshed);
    connect(mNmcliInterface, &NmcliInterface::wifiConnected, this, &NetworkInterface::onWifiConnected);
}

NetworkInterface::WifiInfoList NetworkInterface::wifis()
{
    return QQmlListProperty<WifiInfo>(this, nullptr,
                                      &NetworkInterface::networkCount,
                                      &NetworkInterface::networkAt);
}

bool NetworkInterface::isRunning()
{
    return mWifiReadProc && (mWifiReadProc->state() == QProcess::Starting
                             || mWifiReadProc->state() == QProcess::Running);
}

QString NetworkInterface::connectedSsid() const
{
    return (mConnectedWifiInfo ? mConnectedWifiInfo->mSsid : "");
}

void NetworkInterface::refereshWifis(bool forced)
{
    if (mNmcliInterface->isRunning()) {
        return;
    }

    mNmcliInterface->refreshWifis(forced);
}

void NetworkInterface::connectWifi(WifiInfo* wifiInfo, const QString& password)
{
    if (!wifiInfo || mNmcliInterface->isRunning()) {
        return;
    }

    mRequestedToConnectedWifi = wifiInfo;
    mNmcliInterface->connectToWifi(wifiInfo->mBssid, password);
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

void NetworkInterface::onWifiListRefreshed(const QList<QMap<QString, QVariant>>& wifis)
{
    //! Wifi list
    QList<WifiInfo*> wifiInfos;

    for (const auto& wifi : wifis) {
        wifiInfos.push_back(new WifiInfo(
            wifi["inUse"].toBool(),
            wifi["ssid"].toString(),
            wifi["bssid"].toString(),
            wifi["signal"].toInt(),
            wifi["security"].toString()
        ));
    }

    //! Update mNetworks
    qDeleteAll(mWifiInfos);
    //! Set mConnectedWifiInfo since it's not valid anymore
    mConnectedWifiInfo = nullptr;

    mWifiInfos.clear();
    mWifiInfos = std::move(wifiInfos);

    emit wifisChanged();
}

void NetworkInterface::onWifiConnected(const QString& bssid)
{
    if (mConnectedWifiInfo) {
        mConnectedWifiInfo->setProperty("connected", false);
    }

    if (mRequestedToConnectedWifi) {
        mRequestedToConnectedWifi->setProperty("connected", true);

        mConnectedWifiInfo = mRequestedToConnectedWifi;
        mRequestedToConnectedWifi = nullptr;
        emit connectedSsidChanged();
    }

}

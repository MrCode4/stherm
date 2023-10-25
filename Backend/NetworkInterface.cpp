#include "NetworkInterface.h"
#include "NmcliInterface.h"

#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>

NetworkInterface::NetworkInterface(QObject *parent)
    : QObject{parent}
    , mNmcliInterface { new NmcliInterface(this) }
    , mConnectedWifiInfo { nullptr }
    , mRequestedToConnectedWifi { nullptr }
    , mDeviceIsOn { false }
{
    connect(mNmcliInterface, &NmcliInterface::wifiListRefereshed, this, &NetworkInterface::onWifiListRefreshed);
    connect(mNmcliInterface, &NmcliInterface::wifiConnected, this, &NetworkInterface::onWifiConnected);
    connect(mNmcliInterface, &NmcliInterface::wifiDisconnected, this, &NetworkInterface::onWifiDisconnected);
    connect(mNmcliInterface, &NmcliInterface::isRunningChanged, this, &NetworkInterface::isRunningChanged);
    connect(mNmcliInterface, &NmcliInterface::wifiDevicePowerChanged, this, [&](bool on) {
        if (!on) {
            qDeleteAll(mWifiInfos);
            mWifiInfos.clear();

            emit connectedSsidChanged();
            emit wifisChanged();
        }

        mDeviceIsOn = on;
        emit deviceIsOnChanged();
    });

    connect(mNmcliInterface, &NmcliInterface::wifiForgotten, this, [this](const QString& ssid) {
        if (mConnectedWifiInfo && mConnectedWifiInfo->mSsid == ssid) {
            mConnectedWifiInfo->setProperty("connected", false);

            mConnectedWifiInfo = nullptr;
            emit connectedSsidChanged();
        }
    });
}

NetworkInterface::WifiInfoList NetworkInterface::wifis()
{
    return QQmlListProperty<WifiInfo>(this, nullptr,
                                      &NetworkInterface::networkCount,
                                      &NetworkInterface::networkAt);
}

bool NetworkInterface::isRunning()
{
    return mNmcliInterface && mNmcliInterface->isRunning();
}

QString NetworkInterface::connectedSsid() const
{
    return (mConnectedWifiInfo ? mConnectedWifiInfo->mSsid : "");
}

void NetworkInterface::refereshWifis(bool forced)
{
    if (isRunning()) {
        return;
    }

    mNmcliInterface->refreshWifis(forced);
}

void NetworkInterface::connectWifi(WifiInfo* wifiInfo, const QString& password)
{
    if (!wifiInfo || isRunning()) {
        return;
    }

    mRequestedToConnectedWifi = wifiInfo;
    mNmcliInterface->connectToWifi(wifiInfo->mBssid, password);
}

void NetworkInterface::disconnectWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || isRunning()) {
        return;
    }
    
    mNmcliInterface->disconnectFromWifi(wifiInfo->mSsid);
}

void NetworkInterface::forgetWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || isRunning()) {
        return;
    }

    mNmcliInterface->forgetWifi(wifiInfo->mSsid);
}

void NetworkInterface::turnOn()
{
    if (isRunning()) {
        return;
    }

    mNmcliInterface->turnWifiDeviceOn();
}

void NetworkInterface::turnOff()
{
    if (isRunning()) {
        return;
    }

    mNmcliInterface->turnWifiDeviceOff();
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
    //! Set mConnectedWifiInfo since it will not be valid anymore
    mConnectedWifiInfo = nullptr;

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

        if (wifi["inUse"].toBool()) {
            mConnectedWifiInfo = wifiInfos.back();
        }
    }

    //! Update mNetworks
    qDeleteAll(mWifiInfos);

    mWifiInfos.clear();
    mWifiInfos = std::move(wifiInfos);

    emit connectedSsidChanged();
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

void NetworkInterface::onWifiDisconnected()
{
    if (mConnectedWifiInfo) {
        mConnectedWifiInfo->setProperty("connected", false);

        mConnectedWifiInfo = nullptr;
        emit connectedSsidChanged();
    }
}

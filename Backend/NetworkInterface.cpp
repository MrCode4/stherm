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
    connect(mNmcliInterface, &NmcliInterface::errorOccured, this, &NetworkInterface::onErrorOccured);
    connect(mNmcliInterface, &NmcliInterface::wifiListRefereshed, this, &NetworkInterface::onWifiListRefreshed);
    connect(mNmcliInterface, &NmcliInterface::wifiConnected, this, &NetworkInterface::onWifiConnected);
    connect(mNmcliInterface, &NmcliInterface::wifiDisconnected, this, &NetworkInterface::onWifiDisconnected);
    connect(mNmcliInterface, &NmcliInterface::isRunningChanged, this, &NetworkInterface::isRunningChanged);
    connect(mNmcliInterface, &NmcliInterface::wifiDevicePowerChanged, this, [&](bool on) {
        if (mDeviceIsOn != on) {
            mDeviceIsOn = on;
            emit deviceIsOnChanged();
        }

        emit connectedWifiChanged();
        emit wifisChanged();
    });

    connect(mNmcliInterface, &NmcliInterface::wifiForgotten, this, [this](const QString& ssid) {
        if (mConnectedWifiInfo && mConnectedWifiInfo->mSsid == ssid) {
            mConnectedWifiInfo->setProperty("connected", false);

            mConnectedWifiInfo = nullptr;
            emit connectedWifiChanged();
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

WifiInfo* NetworkInterface::connectedWifi() const
{
    return (mDeviceIsOn ? mConnectedWifiInfo : nullptr);
}

void NetworkInterface::refereshWifis(bool forced)
{
    if (isRunning() || !mDeviceIsOn) {
        return;
    }

    mNmcliInterface->refreshWifis(forced);
}

void NetworkInterface::connectWifi(WifiInfo* wifiInfo, const QString& password)
{
    if (!wifiInfo || isRunning() || !mDeviceIsOn) {
        return;
    }

    mRequestedToConnectedWifi = wifiInfo;
    mNmcliInterface->connectToWifi(wifiInfo->mBssid, password);
}

void NetworkInterface::connectSavedWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || isRunning() || !mDeviceIsOn) {
        return;
    }

    mRequestedToConnectedWifi = wifiInfo;
    mNmcliInterface->connectSavedWifi(wifiInfo->mSsid, wifiInfo->mBssid);
}

void NetworkInterface::disconnectWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || isRunning() || !mDeviceIsOn) {
        return;
    }
    
    mNmcliInterface->disconnectFromWifi(wifiInfo->mSsid);
}

void NetworkInterface::forgetWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || isRunning() || !mDeviceIsOn) {
        return;
    }

    mNmcliInterface->forgetWifi(wifiInfo->mSsid);
}

bool NetworkInterface::isWifiSaved(WifiInfo* wifiInfo)
{
    if (!wifiInfo) {
        return false;
    }

    return mNmcliInterface->hasWifiProfile(wifiInfo->mSsid, wifiInfo->mBssid);
}

void NetworkInterface::turnOn()
{
    if (isRunning() || mDeviceIsOn) {
        return;
    }

    mNmcliInterface->turnWifiDeviceOn();
}

void NetworkInterface::turnOff()
{
    if (isRunning() || !mDeviceIsOn) {
        return;
    }

    mNmcliInterface->turnWifiDeviceOff();
}

void NetworkInterface::addConnection(const QString& name,
                                     const QString& ssid,
                                     const QString& ip4,
                                     const QString& gw4,
                                     const QString& dns,
                                     const QString& security,
                                     const QString& password)
{
    if (isRunning() || !mDeviceIsOn) {
        return;
    }

    mNmcliInterface->addConnection(name, ssid, ip4, gw4, dns, security, password);
}

WifiInfo* NetworkInterface::networkAt(WifiInfoList* list, qsizetype index)
{
    if (NetworkInterface* ni = qobject_cast<NetworkInterface*>(list->object)) {
        if (ni->mDeviceIsOn && index >= 0 && index < ni->mWifiInfos.count()) {
            return ni->mWifiInfos.at(index);
        }
    }

    return nullptr;
}

qsizetype NetworkInterface::networkCount(WifiInfoList* list)
{
    if (NetworkInterface* ni = qobject_cast<NetworkInterface*>(list->object)) {
        return ni->mDeviceIsOn ? ni->mWifiInfos.count() : 0;
    }

    return 0;
}

void NetworkInterface::onErrorOccured(int error)
{
    switch (error) {
    case NmcliInterface::Error::ActivationFailed:
        //! If there was a wifi requested to connect, forget it
        if (mRequestedToConnectedWifi) {
            forgetWifi(mRequestedToConnectedWifi);
            mRequestedToConnectedWifi = nullptr;
        }
        break;
    default:
        qDebug() << Q_FUNC_INFO << __LINE__ << " nmcli Error: " << error;
        break;
    }
}

void NetworkInterface::onWifiListRefreshed(const QList<QMap<QString, QVariant>>& wifis)
{
    //! Find wifis to be deleted.
    QList<WifiInfo*> toDeleteWifis;
    for (WifiInfo* wi : mWifiInfos) {
        auto wiInWifis = std::find_if(wifis.begin(), wifis.end(), [&](const QMap<QString, QVariant>& wf) {
            return wi->mBssid == wf["bssid"].toString();
        });

        if (wiInWifis == wifis.end()) {
            //! This should be deleted
            toDeleteWifis.push_back(wi);
        }
    }

    bool anyWifiConnected = false;
    //! Wifi list
    QList<WifiInfo*> wifiInfos;

    for (const auto& wifi : wifis) {
        const QString bssid = wifi["bssid"].toString();

        //! Check if this bssid is in mWifiInfos
        auto wiInstance = std::find_if(mWifiInfos.begin(), mWifiInfos.end(), [&](WifiInfo* w) {
            return w->mBssid == bssid;
        });

        if (wiInstance == mWifiInfos.end()) {
            WifiInfo* newWifi = new WifiInfo(
                                    wifi["inUse"].toBool(),
                                    wifi["ssid"].toString(),
                                    wifi["bssid"].toString(),
                                    wifi["signal"].toInt(),
                                    wifi["security"].toString()
                );
            wifiInfos.push_back(newWifi);
        } else {
            (*wiInstance)->setProperty("connected", wifi["inUse"].toBool());
            (*wiInstance)->setProperty("strength", wifi["signal"].toInt());
            wifiInfos.push_back((*wiInstance));
        }

        if (wifi["inUse"].toBool() && !anyWifiConnected) {
            anyWifiConnected = true;
            mConnectedWifiInfo = wifiInfos.back();
        }
    }

    //! Just clear mWifiInfos, don't delete instance as ownership is transfered to js
    mWifiInfos.clear();
    mWifiInfos = std::move(wifiInfos);

    if (!anyWifiConnected
        || std::find(toDeleteWifis.begin(), toDeleteWifis.end(), mConnectedWifiInfo) != toDeleteWifis.end()) {
        mConnectedWifiInfo = nullptr;
    }
    emit connectedWifiChanged();
    emit wifisChanged();

    //! Now delete not needed wifis
    qDeleteAll(toDeleteWifis);
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
        emit connectedWifiChanged();
    }
}

void NetworkInterface::onWifiDisconnected()
{
    if (mConnectedWifiInfo) {
        mConnectedWifiInfo->setProperty("connected", false);

        mConnectedWifiInfo = nullptr;
        emit connectedWifiChanged();
    }
}

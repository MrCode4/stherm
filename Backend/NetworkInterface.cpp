#include "NetworkInterface.h"
#include "LogHelper.h"
#include "Nmcli/NmcliInterface.h"
#include "Nmcli/NmcliObserver.h"

#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QNetworkReply>

NetworkInterface::NetworkInterface(QObject *parent)
    : QObject{parent}
    , mNmcliInterface { new NmcliInterface(this) }
    , mNmcliObserver { new NmcliObserver(this) }
    , mConnectedWifiInfo { nullptr }
    , mRequestedToConnectedWifi { nullptr }
    , mDeviceIsOn { false }
    , mHasInternet { false }
    , mNamIsRunning { false }
    , cCheckInternetAccessUrl { QUrl(qEnvironmentVariable("NMCLI_INTERNET_ACCESS_URL",
                                                          "http://google.com")) }
{
    connect(mNmcliInterface, &NmcliInterface::errorOccured, this, &NetworkInterface::onErrorOccured);
    connect(mNmcliInterface, &NmcliInterface::wifiListRefereshed, this, &NetworkInterface::onWifiListRefreshed);
    connect(mNmcliInterface, &NmcliInterface::isRunningChanged, this, &NetworkInterface::isRunningChanged);
    connect(mNmcliObserver, &NmcliObserver::wifiConnected, this, &NetworkInterface::onWifiConnected);
    connect(mNmcliObserver, &NmcliObserver::wifiDisconnected, this, &NetworkInterface::onWifiDisconnected);
    connect(mNmcliObserver, &NmcliObserver::wifiDevicePowerChanged, this, [&](bool on) {
        if (mDeviceIsOn != on) {
            mDeviceIsOn = on;

            emit deviceIsOnChanged();
            emit wifisChanged();
            emit connectedWifiChanged();
        }
    });

    connect(mNmcliObserver, &NmcliObserver::wifiForgotten, this, [this](const QString& ssid) {
        if (mConnectedWifiInfo && mConnectedWifiInfo->mSsid == ssid) {
            setConnectedWifiInfo(nullptr);
        }
    });

    connect(mNmcliObserver, &NmcliObserver::wifiIsConnecting, this, [this](const QString ssid) {
        //! Search for a wifi with this bssid.
        for (WifiInfo* wifi : mWifiInfos) {
            if (wifi->mSsid == ssid) {
                wifi->setProperty("isConnecting", true);
                return;
            }
        }
    });

    //! Connecting to QNetworkAccessManager
    connect(&mNam, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {

        bool hasInternet = reply->error() == QNetworkReply::NoError;
        setHasInternet(hasInternet);

        mNamIsRunning = false;
        reply->deleteLater();
    });

    //! Set up time for checking internet access: every 30 seconds
    mCheckInternetAccessTmr.setInterval(cCheckInternetAccessInterval);
    connect(&mCheckInternetAccessTmr, &QTimer::timeout, this, &NetworkInterface::checkHasInternet);
    connect(this, &NetworkInterface::connectedWifiChanged, this, [&]() {
        if (mConnectedWifiInfo) {
            if (!mCheckInternetAccessTmr.isActive()) {
                mCheckInternetAccessTmr.start();

                checkHasInternet();
            }
        } else {
            if (mCheckInternetAccessTmr.isActive()) {
                mCheckInternetAccessTmr.stop();

                setHasInternet(false);
            }
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

void NetworkInterface::checkHasInternet()
{
    if (!mConnectedWifiInfo) {
        setHasInternet(false);
    } else if (!mNamIsRunning) {
        QNetworkRequest request(cCheckInternetAccessUrl);
        request.setTransferTimeout(8000);
        mNamIsRunning = true;

        mNam.get(request);
    }
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
        || std::find(toDeleteWifis.begin(),
                     toDeleteWifis.end(),
                     mConnectedWifiInfo) != toDeleteWifis.end()) {
        mConnectedWifiInfo = nullptr;
    }
    emit connectedWifiChanged();
    emit wifisChanged();

    //! Now delete not needed wifis
    qDeleteAll(toDeleteWifis);
}

void NetworkInterface::onWifiConnected(const QString& ssid)
{
    if (mRequestedToConnectedWifi && mRequestedToConnectedWifi->mBssid == ssid) {
        setConnectedWifiInfo(mRequestedToConnectedWifi);
    } else {
        mRequestedToConnectedWifi = nullptr;
        //! Search for a wifi with this bssid.
        for (WifiInfo* wifi : mWifiInfos) {
            if (wifi->mBssid == ssid) {
                setConnectedWifiInfo(wifi);
                return;
            }
        }
    }
}

void NetworkInterface::onWifiDisconnected()
{
    setConnectedWifiInfo(nullptr);
}

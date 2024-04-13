#include "NetworkInterface.h"
#include "LogHelper.h"
#include "Nmcli/NmcliInterface.h"

#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QNetworkReply>

NetworkInterface::NetworkInterface(QObject *parent)
    : QObject{parent}
    , mNmcliInterface { new NmcliInterface(this) }
    , mWifiInfos { mNmcliInterface->getWifis() }
    , mConnectedWifiInfo { nullptr }
    , mRequestedToConnectedWifi { nullptr }
    , mDeviceIsOn { false }
    , mHasInternet { false }
    , mNamIsRunning { false }
    , cCheckInternetAccessUrl { QUrl(qEnvironmentVariable("NMCLI_INTERNET_ACCESS_URL",
                                                          "http://google.com")) }
{
    connect(mNmcliInterface, &NmcliInterface::errorOccured, this,
            &NetworkInterface::onErrorOccured);
    connect(mNmcliInterface, &NmcliInterface::isRunningChanged, this,
            &NetworkInterface::isRunningChanged);
    connect(mNmcliInterface, &NmcliInterface::connectedWifiChanged, this,
            &NetworkInterface::connectedWifiChanged);
    connect(mNmcliInterface, &NmcliInterface::wifisChanged, this, &NetworkInterface::wifisChanged);

    //! Connecting to QNetworkAccessManager
    connect(&mNam, &QNetworkAccessManager::finished, this, [&](QNetworkReply* reply) {

        bool hasInternet = reply->error() == QNetworkReply::NoError;
        mNamIsRunning = false;

        if (hasInternet) {
            mSetNoInternetTimer.stop();
            setHasInternet(true);
        } else {
            TRACE << "check has internet has error" << mHasInternet << mSetNoInternetTimer.isActive() << reply->error();
            // sending a sooner request when we previously had internet but we get failed
            if (mHasInternet)
                QTimer::singleShot(10000, this, &NetworkInterface::checkHasInternet);
            // setting no Internet after a double time period to let the blip go away
            if (!mSetNoInternetTimer.isActive())
                mSetNoInternetTimer.start();
        }

        reply->deleteLater();
    });

    //! Set up time for checking internet access: every 30 seconds
    mCheckInternetAccessTmr.setInterval(cCheckInternetAccessInterval);
    connect(&mCheckInternetAccessTmr, &QTimer::timeout, this, &NetworkInterface::checkHasInternet);
    connect(this, &NetworkInterface::connectedWifiChanged, this, [&]() {
        mSetNoInternetTimer.stop();
        if (mConnectedWifiInfo) {
            if (!mCheckInternetAccessTmr.isActive()) {
                mCheckInternetAccessTmr.start();
            }

            checkHasInternet();
        } else {
            if (mCheckInternetAccessTmr.isActive()) {
                mCheckInternetAccessTmr.stop();
            }

            setHasInternet(false);
        }
    });

    mSetNoInternetTimer.setInterval(cCheckInternetAccessInterval * 2);
    mSetNoInternetTimer.setSingleShot(true);
    connect(&mSetNoInternetTimer, &QTimer::timeout, this, [this](){
        TRACE << "no internet found during last" << cCheckInternetAccessInterval * 2 / 1000 << "seconds."
              << "settings no internet";
        setHasInternet(false);
    });
}

NetworkInterface::WifisQmlList NetworkInterface::wifis()
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
    return (mDeviceIsOn && mNmcliInterface ? mNmcliInterface->connectedWifi() : nullptr);
}

QString NetworkInterface::ipv4Address() const
{
    for (const QNetworkInterface &netInterface : QNetworkInterface::allInterfaces()) {
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        if((flags & QNetworkInterface::IsRunning) && !(flags & QNetworkInterface::IsLoopBack)){
            for (const QNetworkAddressEntry &address : netInterface.addressEntries()) {
                if(address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                    return address.ip().toString();
            }
        }
    }

    return "Not available";
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
    mNmcliInterface->connectToWifi(wifiInfo, password);
}

void NetworkInterface::disconnectWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || isRunning() || !mDeviceIsOn) {
        return;
    }
    
    mNmcliInterface->disconnectFromWifi(wifiInfo);
}

void NetworkInterface::forgetWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || isRunning() || !mDeviceIsOn) {
        return;
    }

    mNmcliInterface->forgetWifi(wifiInfo);
}

bool NetworkInterface::isWifiSaved(WifiInfo* wifiInfo)
{
    if (!wifiInfo) {
        return false;
    }

    return mNmcliInterface->hasWifiProfile(wifiInfo);
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

WifiInfo* NetworkInterface::networkAt(WifisQmlList* list, qsizetype index)
{
    if (NetworkInterface* ni = qobject_cast<NetworkInterface*>(list->object)) {
        if (ni->mDeviceIsOn && index >= 0 && index < ni->mWifiInfos.count()) {
            return ni->mWifiInfos.at(index);
        }
    }

    return nullptr;
}

qsizetype NetworkInterface::networkCount(WifisQmlList* list)
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
    default:
        qDebug() << Q_FUNC_INFO << __LINE__ << " nmcli Error: " << error;
        break;
    }
}

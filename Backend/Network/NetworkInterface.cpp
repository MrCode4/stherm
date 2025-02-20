#include "NetworkInterface.h"

#include "LogHelper.h"
#include "Core/NetworkManager.h"
#include "Nmcli/NmcliInterface.h"

#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QNetworkReply>
#include <QCoreApplication>

#include <QLoggingCategory>
#include "LogHelper.h"

Q_LOGGING_CATEGORY(NetworkInterfaceLogCat, "NetworkInterfaceLog")
#define NI_LOG TRACE_CATEGORY(NetworkInterfaceLogCat)

bool compareWifiStrength(WifiInfo *a, WifiInfo *b) {
    return a->strength() > b->strength();
}


NetworkInterface* NetworkInterface::mMe = nullptr;

NetworkInterface *NetworkInterface::me()
{
    if (!mMe) mMe = new NetworkInterface(qApp);

    return mMe;
}


NetworkInterface::NetworkInterface(QObject *parent)
    : QObject{parent}
    , mNmcliInterface { new NmcliInterface(this) }
    , mWifiInfos { mNmcliInterface->getWifis() }
    , mRequestedToConnectedWifi { nullptr }
    , mHasInternet { false }
    , mNamIsRunning { false }
    , cCheckInternetAccessUrl { QUrl(qEnvironmentVariable("NMCLI_INTERNET_ACCESS_URL",
                                                          "http://google.com")) }
    , mForgettingWifis { false }
    , mIsWifiDisconnectedManually { false }
    , mIsBusyAutoConnection { false }
{
    connect(mNmcliInterface, &NmcliInterface::deviceIsOnChanged, this,
            &NetworkInterface::deviceIsOnChanged);
    connect(mNmcliInterface, &NmcliInterface::errorOccured, this,
            &NetworkInterface::onErrorOccured);
    connect(mNmcliInterface, &NmcliInterface::busyRefreshingChanged,
            this, &NetworkInterface::busyRefreshingChanged);
    connect(mNmcliInterface, &NmcliInterface::busyChanged, this,
            &NetworkInterface::busyChanged);
    connect(mNmcliInterface, &NmcliInterface::connectedWifiChanged, this,
            &NetworkInterface::connectedWifiChanged);
    connect(mNmcliInterface, &NmcliInterface::wifisChanged, this, &NetworkInterface::wifisChanged);
    connect(mNmcliInterface, &NmcliInterface::wifiNeedAuthentication, this,
            &NetworkInterface::incorrectWifiPassword);
    connect(mNmcliInterface, &NmcliInterface::wifiForgotten, this,
            &NetworkInterface::wifiForgotten);

    //! Set up time for checking internet access: every 30 seconds
    mCheckInternetAccessTmr.setInterval(cCheckInternetAccessInterval);
    connect(&mCheckInternetAccessTmr, &QTimer::timeout, this, &NetworkInterface::checkHasInternet);
    connect(this, &NetworkInterface::connectedWifiChanged, this, [&]() {

        mSetNoInternetTimer.stop();

        // clear the cache to restore internet access faster
        clearDNSCache();

        auto connectedWifiInfo = connectedWifi();
        if (connectedWifiInfo) {
            // Set to false to start auto connection if needed.
            setIsWifiDisconnectedManually(false);

            mCheckInternetAccessTmr.start();
            checkHasInternet();

        } else {
            printWifisInformation("Wi-Fi disconnected.");
            if (!mIsWifiDisconnectedManually)
                mAutoConnectToWifiTimer.start();

            if (mCheckInternetAccessTmr.isActive()) {
                mCheckInternetAccessTmr.stop();
            }

            setHasInternet(false);
        }
    });

    mSetNoInternetTimer.setInterval(cCheckInternetAccessInterval * 2);
    mSetNoInternetTimer.setSingleShot(true);
    connect(&mSetNoInternetTimer, &QTimer::timeout, this, [this](){
        NI_LOG << "no internet found during last" << cCheckInternetAccessInterval * 2 / 1000 << "seconds."
              << "settings no internet";
        setHasInternet(false);
    });

    connect(mNmcliInterface, &NmcliInterface::ciphersAreReady, this, [this]() {
        this->mDoesDeviceSupportWPA3 = checkWPA3Support();
    });

    connect(this, &NetworkInterface::busyChanged, this, [this]() {
        if (mForgettingWifis && !busy())
            processForgettingWiFis();
    });

    connect(mNmcliInterface, &NmcliInterface::autoConnectSavedInRangeWifiFinished, this, [this](WifiInfo *wifi) {
        NI_LOG << "Auto connection Finished ...";
        if (wifi)
            NI_LOG << "Auto connection for " << wifi->wifiInformation();
        tryConnectToSavedInrangeWifi(wifi);
    });

    mAutoConnectToWifiTimer.setInterval(2 * 60 * 1000);
    mAutoConnectToWifiTimer.setSingleShot(false);
    connect(&mAutoConnectToWifiTimer, &QTimer::timeout, this, [this]() {
        if (connectedWifi()) {
            mAutoConnectToWifiTimer.stop();
            return;
        }

        printWifisInformation("Auto connect starting.");

        if (mIsWifiDisconnectedManually) {
            mAutoConnectToWifiTimer.stop();
            NI_LOG << "Auto connection Stopped due to manually disconnected from wifi.";
            return;
        }

        // Restart the timer if the mNmcliInterface is busy
        if (mNmcliInterface->busy() || mIsBusyAutoConnection) {
            mAutoConnectToWifiTimer.start();
            NI_LOG << "Auto connection restarted.";
            return;
        }


        // Update the auto connection wifi list:
        std::copy_if(mWifiInfos.begin(), mWifiInfos.end(),
                     std::back_inserter(mAutoConnectSavedInRangeWifis), [&](WifiInfo* obj) {
                         return obj->ssid().isEmpty() == false && obj->isSaved() && obj->strength() > -1;
                     });

        if (mAutoConnectSavedInRangeWifis.empty()) {
            mAutoConnectToWifiTimer.stop();
            NI_LOG << "Auto connection Stopped.";

        } else {
            NI_LOG << "Auto connection started.";
            std::sort(mAutoConnectSavedInRangeWifis.begin(), mAutoConnectSavedInRangeWifis.end(), compareWifiStrength);
            tryConnectToSavedInrangeWifi();
        }

    });
    mAutoConnectToWifiTimer.start();

    QTimer::singleShot(10 * 60 * 1000, this, [this]() {
        printWifisInformation("10 minutes passed.");
    });
}


void NetworkInterface::tryConnectToSavedInrangeWifi(WifiInfo *triedWifi) {
    if (connectedWifi()) {
        mAutoConnectToWifiTimer.stop();
        mIsBusyAutoConnection = false;
        return;
    }

    if (triedWifi) {
        mAutoConnectSavedInRangeWifis.removeOne(triedWifi);
    }

    if (mAutoConnectSavedInRangeWifis.empty()) {
        NI_LOG << "No saved inrange wifis for auto connection.";
        mIsBusyAutoConnection = false;

    } else {
        mIsBusyAutoConnection = true;
        auto wifi = mAutoConnectSavedInRangeWifis.front();
        if (!mNmcliInterface->autoConnectSavedWifiAsync(wifi)) {
            mAutoConnectSavedInRangeWifis.pop_front();
            tryConnectToSavedInrangeWifi();
        }
    }
}

QString NetworkInterface::refreshWiFiResult() const {
    return mNmcliInterface->iwOut();
}

NetworkInterface::WifisQmlList NetworkInterface::wifis()
{
    return QQmlListProperty<WifiInfo>(this, nullptr,
                                      &NetworkInterface::networkCount,
                                      &NetworkInterface::networkAt);
}

WifiInfo* NetworkInterface::connectedWifi() const
{
    return (mNmcliInterface->isDeviceOn() && mNmcliInterface ? mNmcliInterface->connectedWifi() : nullptr);
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
    if (!mNmcliInterface->isDeviceOn()) {
        return;
    }

    mNmcliInterface->refreshWifis(forced);
}

void NetworkInterface::connectWifi(WifiInfo* wifiInfo, const QString& password)
{
    if (!wifiInfo || wifiInfo->connected() || wifiInfo->isConnecting() || mNmcliInterface->busy() || mForgettingWifis) {
        return;
    }

    // Set to false to start auto connection if needed.
    setIsWifiDisconnectedManually(false);

    mRequestedToConnectedWifi = wifiInfo;
    mNmcliInterface->connectToWifi(wifiInfo, password);
}

void NetworkInterface::disconnectWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || !wifiInfo->connected() || mNmcliInterface->busy()) {
        return;
    }

    setIsWifiDisconnectedManually(true);
    mNmcliInterface->disconnectFromWifi(wifiInfo);
}

void NetworkInterface::forgetWifi(WifiInfo* wifiInfo)
{
    if (!wifiInfo || !mNmcliInterface->isDeviceOn()) {
        NI_LOG << "Worst case scenario: Error in forgetWifi" << wifiInfo << mNmcliInterface->isDeviceOn();
        emit  wifiForgotten(wifiInfo);
        return;
    }

    if (wifiInfo->connected()) {
        setIsWifiDisconnectedManually(true);
    }

    mNmcliInterface->forgetWifi(wifiInfo);
}

void NetworkInterface::forgetAllWifis() {
    NI_LOG << "Forget all Wi-Fis started.";
    setIsWifiDisconnectedManually(true);

    processForgettingWiFis();
}

void NetworkInterface::processForgettingWiFis() {
    setForgettingWifis(true);

    auto connectedWiFi = connectedWifi();
    if (connectedWiFi) {
        NI_LOG << "Disconnect from " << connectedWiFi->ssid();
        disconnectWifi(connectedWiFi);
    } else {
        QList <WifiInfo *> forgettingSavedWifis;
        std::copy_if(mWifiInfos.begin(), mWifiInfos.end(),
                     std::back_inserter(forgettingSavedWifis), [](WifiInfo* obj) {
                         return obj->isSaved();
                     });

        setForgettingWifis(!forgettingSavedWifis.empty());

        if (!forgettingSavedWifis.empty()) {
            auto forgetWF = forgettingSavedWifis.first();
            NI_LOG << "Forget Wi-Fi with ssid " << forgetWF->ssid();
            forgetWifi(forgetWF);
            if (!busy()) {
                NI_LOG << "Worst case scenario: isBusy forgetting is false.";
            }
        }
    }

    if (mForgettingWifis == false) {
        emit allWiFiNetworksForgotten();
    }
}

void NetworkInterface::setForgettingWifis(const bool &forgettingWifis) {
    if (mForgettingWifis == forgettingWifis)
        return;

    mForgettingWifis = forgettingWifis;
    emit forgettingAllWifisChanged();

}

void NetworkInterface::setIsWifiDisconnectedManually(const bool &isWifiDisconnectedManually)
{
    NI_LOG << "WiFi disconnected manually: " << isWifiDisconnectedManually;
    if (isWifiDisconnectedManually) {
         mAutoConnectToWifiTimer.stop();

    } else {
        // Start the auto connection for try to connect when the manual connection failed.
        mAutoConnectToWifiTimer.start();
    }

    if (mIsWifiDisconnectedManually == isWifiDisconnectedManually) {
        return;
    }

    mIsWifiDisconnectedManually = isWifiDisconnectedManually;
    emit isWifiDisconnectedManuallyChanged();
}

bool NetworkInterface::forgettingAllWifis() {
    return mForgettingWifis;
}

void NetworkInterface::printWifisInformation(const QString &due)
{
    NI_LOG << "printWifisInformation started due to: "  << due;

    foreach (auto wifi, mWifiInfos) {
        if (wifi)
            NI_LOG << wifi->wifiInformation();
    }
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
    if (mNmcliInterface->isDeviceOn()) {
        return;
    }

    mNmcliInterface->turnWifiDeviceOn();
}

void NetworkInterface::turnOff()
{
    if (!mNmcliInterface->isDeviceOn()) {
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
    if (!mNmcliInterface->isDeviceOn()) {
        return;
    }

    mNmcliInterface->addConnection(name, ssid, ip4, gw4, dns, security, password);
}

WifiInfo* NetworkInterface::networkAt(WifisQmlList* list, qsizetype index)
{
    if (NetworkInterface* ni = qobject_cast<NetworkInterface*>(list->object)) {
        if (ni->mNmcliInterface->isDeviceOn() && index >= 0 && index < ni->mWifiInfos.count()) {
            return ni->mWifiInfos.at(index);
        }
    }

    return nullptr;
}

qsizetype NetworkInterface::networkCount(WifisQmlList* list)
{
    if (NetworkInterface* ni = qobject_cast<NetworkInterface*>(list->object)) {
        return ni->mNmcliInterface->isDeviceOn() ? ni->mWifiInfos.count() : 0;
    }

    return 0;
}

bool NetworkInterface::checkWPA3Support()
{
    QStringList deviceSupportedCiphersList = mNmcliInterface->getDeviceSupportedCiphersList();

    for (const QString &cipher : deviceSupportedCiphersList) {
        if (cipher.contains("GCMP", Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

void NetworkInterface::checkHasInternet()
{
    auto connectedWifiInfo = connectedWifi();
    NI_LOG << "Checking the internet connectivity, " << mNamIsRunning;

    if (!connectedWifiInfo) {
        setHasInternet(false);
    }
    else if (!mNamIsRunning) {
        NI_LOG << "connectedWifiInfo: " << connectedWifiInfo->wifiInformation();

        mNamIsRunning = true;
        QNetworkRequest request(cCheckInternetAccessUrl);
        request.setTransferTimeout(8000);
        auto* reply = NetworkManager::instance()->get(request);
        if (!reply) return;
        reply->ignoreSslErrors();
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            bool hasInternet = reply->error() == QNetworkReply::NoError;
            mNamIsRunning = false;

            if (hasInternet) {
                mSetNoInternetTimer.stop();
                setHasInternet(true);
            } else {
                NI_LOG << "check has internet has error" << mHasInternet << mSetNoInternetTimer.isActive() << reply->error();
                // sending a sooner request when we previously had internet but we get failed
                if (mHasInternet) {
                    QTimer::singleShot(10000, this, &NetworkInterface::checkHasInternet);
                }
                // setting no Internet after a double time period to let the blip go away
                if (!mSetNoInternetTimer.isActive()) {
                    mSetNoInternetTimer.start();
                }
            }

            reply->deleteLater();
        });
    } else {
        NI_LOG << "connectedWifiInfo: " << connectedWifiInfo->wifiInformation();
    }
}

void NetworkInterface::clearDNSCache()
{
    NetworkManager::instance()->clearCache();
}

bool NetworkInterface::doesDeviceSupportWPA3() const
{
    return mDoesDeviceSupportWPA3;
}

void NetworkInterface::onErrorOccured(int error)
{
    switch (error) {
    default:
        NI_LOG << error;
        break;
    }
}

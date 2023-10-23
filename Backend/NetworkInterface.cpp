#include "NetworkInterface.h"

#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>

NetworkInterface::NetworkInterface(QObject *parent)
    : QObject{parent}
    , mWifiReadProc { nullptr }
    , mConnectedWifiInfo { nullptr }
    , mRequestedToConnectedWifi { nullptr }
{
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
    if (!mWifiReadProc) {
        mWifiReadProc = new QProcess(this);
        mWifiReadProc->setReadChannel(QProcess::StandardOutput);

        //! Connect QProcess status changed to isRunningChanged();
        connect(mWifiReadProc, &QProcess::stateChanged, this, &NetworkInterface::isRunningChanged);
    }

    if (isRunning()) {
        return;
    }

    connect(mWifiReadProc, &QProcess::finished, this, &NetworkInterface::onWifiProcessFinished,
            Qt::SingleShotConnection);

#ifdef Q_OS_LINUX
    mWifiReadProc->start("nmcli", { "d",
                                      "wifi",
                                      "list",
                                      "--rescan",
                                      (forced ? "yes" : "auto")
                                  } );

#elif defined Q_OS_WIN32
    //! Use commands suitable for Windows
#endif
}

void NetworkInterface::connectWifi(WifiInfo* wifiInfo, const QString& password)
{
    if (!wifiInfo || wifiInfo->mConnected) {
        return;
    }

    if (!mWifiReadProc || isRunning()) {
        return;
    }

    mRequestedToConnectedWifi = wifiInfo;
    connect(mWifiReadProc, &QProcess::finished, this, &NetworkInterface::onWifiConnectFinished,
            Qt::SingleShotConnection);

#ifdef Q_OS_LINUX
    mWifiReadProc->start("nmcli", { "d",
                                      "wifi",
                                      "connect",
                                      wifiInfo->mSsid,
                                      "password",
                                      password,
                                   });
#elif defined Q_OS_WIN32
#endif
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

void NetworkInterface::onWifiProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0) {
        emit errorOccured("Error in scanning wifi networks: ", "");
        return;
    }

#ifdef Q_OS_LINUX
    if (!mWifiReadProc) {
        return;
    }

    const int npcliColumnTrailingSpaces = 2;

    //! Wifi list
    QList<WifiInfo*> wifis;

    //! Read header to determine column sizes which will be used to read values
    QString headerLine = mWifiReadProc->readLine();

    //! Finding SSID column start index and length
    const QRegularExpression ssidRegex(R"((\sSSID\s+))");
    const int ssidStartIndx = headerLine.indexOf(ssidRegex, 0) + 1; //! +1 is for preceding \s
    const int ssidColLength = ssidRegex.match(headerLine,ssidStartIndx - 1).capturedLength()
                              - 1 - npcliColumnTrailingSpaces;

    //! Finding SIGNAL column start index and length
    const QRegularExpression signalRegex(R"(SIGNAL\s+)");
    const int signalStartIndx = headerLine.indexOf(signalRegex, ssidStartIndx);
    const int signalColLength = signalRegex.match(headerLine, signalStartIndx).capturedLength();

    //! Finding SECURITY column start index and length
    const QRegularExpression securityRegex(R"(SECURITY\s+)");
    const int securStartIndx = headerLine.indexOf(securityRegex, ssidStartIndx);
    const int securColLength = signalRegex.match(headerLine, securStartIndx).capturedLength();

    //! Now get wifi info based on above variables

    QString line = mWifiReadProc->readLine();
    while(!line.isEmpty()) {
        const bool isConnected = line.startsWith("*");


        QString ssid = ssidStartIndx >= 0 && ssidStartIndx + ssidColLength < line.size()
                           ? line.sliced(ssidStartIndx, ssidColLength)
                           : "";
        const int strength = signalStartIndx >= 0 && signalStartIndx + signalColLength < line.size()
                                 ? line.sliced(signalStartIndx, signalColLength).toInt()
                                 : 0;
        const QString secur = securStartIndx >= 0 && securStartIndx + securColLength < line.size()
                                  ? line.sliced(securStartIndx, securColLength)
                                  : 0;

        //! Remove trailing whitespaces from ssid
        while (ssid.size() > 0 && ssid.back().isSpace()) {
            ssid.chop(1);
        }

        wifis.append(new WifiInfo(
            isConnected,
            ssid,
            strength,
            secur,
            this
            ));

        if (isConnected) {
            mConnectedWifiInfo = wifis.back();
            emit connectedSsidChanged();
        }

        //! Read next line
        line = mWifiReadProc->readLine();
    }

    //! Update mNetworks
    qDeleteAll(mWifiInfos);
    //! Set mConnectedWifiInfo since it's not valid anymore
    mConnectedWifiInfo = nullptr;

    mWifiInfos.clear();
    mWifiInfos = std::move(wifis);
#elif defined Q_OS_WIN32
    //! Use commands suitable for Windows
#endif

    emit wifisChanged();
}

void NetworkInterface::onWifiConnectFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0) {
        //! Maybe it's needed to set mConnectedWifiInfo's connected property to false.
        emit errorOccured("Error in connecting to wifi network: ", mConnectedWifiInfo->mSsid);
        mRequestedToConnectedWifi = nullptr;
        return;
    }

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

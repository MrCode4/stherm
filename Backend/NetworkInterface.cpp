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
    if (!mWifiReadProc) {
        mWifiReadProc = new QProcess(this);
        mWifiReadProc->setReadChannel(QProcess::StandardOutput);
        connect(mWifiReadProc, &QProcess::finished, this, &NetworkInterface::onWifiProcessFinished);
    }

    if (mWifiReadProc->state() == QProcess::Starting
        || mWifiReadProc->state() == QProcess::Running) {
        return;
    }

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
        qWarning() << "Error in scanning wifis: " << mWifiReadProc->errorString();
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
        const QString ssid = ssidStartIndx >= 0 && ssidStartIndx + ssidColLength < line.size()
                                 ? line.sliced(ssidStartIndx, ssidColLength)
                                 : "";
        const int strength = signalStartIndx >= 0 && signalStartIndx + signalColLength < line.size()
                                 ? line.sliced(signalStartIndx, signalColLength).toInt()
                                 : 0;
        const QString secur = securStartIndx >= 0 && securStartIndx + securColLength < line.size()
                                  ? line.sliced(securStartIndx, securColLength)
                                  : 0;

        wifis.append(new WifiInfo(
            isConnected,
            ssid,
            strength,
            secur,
            this
            ));


        //! Read next line
        line = mWifiReadProc->readLine();
    }

    //! Update mNetworks
    qDeleteAll(mWifiInfos);
    mWifiInfos.clear();
    mWifiInfos = std::move(wifis);
#elif defined Q_OS_WIN32
    //! Use commands suitable for Windows
#endif

    emit wifisChanged();
}

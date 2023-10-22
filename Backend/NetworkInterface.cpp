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

}

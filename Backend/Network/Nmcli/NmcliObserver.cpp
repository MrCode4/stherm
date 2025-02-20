#include "NmcliObserver.h"

#include "LogHelper.h"
#include "Nmcli.h"

NmcliObserver::NmcliObserver(QObject *parent)
    : QObject{ parent }
    , mDeviceIsOn { false }
    , mCliCommon { new NmCli(this) }
    , mCliMonitor { new NmCli(this) }
{
    //! This lambda gets wifi device name and starts monitoring
    auto startMonitor = [this](QProcess* process) {
        if (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) {
            //! Get wifi device name
            QByteArray line = process->readLine();
            while (!line.isEmpty() && line != "wifi\n") {
                line = process->readLine();
            }

            mWifiDevice = process->readLine();
            if (!mWifiDevice.isEmpty()) {
                mWifiDevice.remove(mWifiDevice.size() - 1, 1);

                //! Start monitoring
                NC_DEBUG << "Network monitoring started";

                mCliMonitor->startMonitoring([this](QProcess* process) {
                    connect(process, &QProcess::readyReadStandardOutput, this,
                            &NmcliObserver::onMonitorProcessReadReady);
                });

                //! Get device enable state
                getDevicePowerState();
                return;
            }
        }

        NC_CRITICAL << "No wifi device is found. Wifi will not function";
    };
    mCliCommon->getWifiDeviceName(startMonitor);
}

NmcliObserver::~NmcliObserver()
{
    mCliCommon->kill();
    mCliMonitor->kill();
}

bool NmcliObserver::isWifiOn() const
{
    return mDeviceIsOn;
}

void NmcliObserver::setDevicePowerState(bool isOn)
{
    if (mDeviceIsOn == isOn) {
        return;
    }

    mDeviceIsOn = isOn;
    emit wifiDevicePowerChanged();
}

void NmcliObserver::getDevicePowerState()
{
    mCliCommon->getDevicePowerState([this] (QProcess* process) {
        if (process->exitCode() == 0) {
            setDevicePowerState(process->readLine().contains("enabled"));
        }
    });
}

void NmcliObserver::onMonitorProcessReadReady()
{
    auto process = qobject_cast<QProcess*>(sender());
    if (!process) {
        qWarning() << "onMonitorProcessReadReady: invalid process instance";
        return;
    }

    QString message = process->readLine();
    message.chop(1); //! Discard \n
    while(!message.isEmpty()) {
        TRACE << "MONITOR, LINE: " << message;
        if (message.endsWith(NC_MSG_CONNECTED)) {
            //! Connected to a wifi
            QString connectedWifiSsid = message.sliced(1, message.length() - NC_MSG_CONNECTED.length() - 2);
            emit wifiConnected(connectedWifiSsid);

            mConnectingWifiSsid = "";
        } else if (message.endsWith(NC_MSG_WIFI_PROFILE_REMOVED)) {
            //! A wifi is forgotten: get its ssid
            QString forgottenSsid = message.sliced(0, message.length()
                                                          - NC_MSG_WIFI_PROFILE_REMOVED.length());
            emit wifiForgotten(forgottenSsid);
        } else if (message.startsWith(mWifiDevice)) {
            //! Messages that starts with wifi device name
            message = message.sliced(mWifiDevice.length());

            if (message.startsWith(NC_MSG_USING_CONNECTION)) {
                //! Network manager is trying to connect to a wifi
                QString wifiSsid = message.sliced(NC_MSG_USING_CONNECTION.length() + 1,
                                                  message.length() - NC_MSG_USING_CONNECTION.length() - 2);

                mConnectingWifiSsid = wifiSsid;
                emit wifiIsConnecting(wifiSsid);
            } else if (message == NC_MSG_DEVICE_OFF) {
                //! Wifi is turned off
                setDevicePowerState(false);
            } else {
                if (!mDeviceIsOn) {
                    //! Device is now on.
                    setDevicePowerState(true);
                }

                if (message == NC_MSG_DISCONNECTED
                    || message == NC_MSG_DISCONNECT_STATE) {
                    //! Wifi is disconnected
                    emit wifiDisconnected();
                } else if (message == NC_MSG_CONNECTION_FAILED && !mConnectingWifiSsid.isEmpty()) {
                    //! Need authentication
                    emit wifiNeedAuthentication(mConnectingWifiSsid);
                    mConnectingWifiSsid = "";
                }
            }
        }

        message = process->readLine();
        message.chop(1); //! Discard \n
    }
}

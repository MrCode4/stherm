#include "NmcliObserver.h"

#include "Nmcli.h"

NmcliObserver::NmcliObserver(QObject *parent)
    : QObject{ parent }
    , mDeviceIsOn { false }
    , mMonitorProcess { new QProcess(this) }
{
    //! Initialize monitoring
    mMonitorProcess->setReadChannel(QProcess::StandardOutput);

    //! This lambda gets wifi device name and starts monitoring
    auto startMonitor = [&](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            //! Get wifi device name
            QByteArray line = mMonitorProcess->readLine();
            while (!line.isEmpty() && line != "wifi\n") {
                line = mMonitorProcess->readLine();
            }

            mWifiDevice = mMonitorProcess->readLine();
            if (!mWifiDevice.isEmpty()) {
                mWifiDevice.remove(mWifiDevice.size() - 1, 1);

                //! Start monitoring
                NC_DEBUG << "Network monitoring started";

                connect(mMonitorProcess, &QProcess::readyReadStandardOutput, this,
                        &NmcliObserver::onMonitorProcessReadReady);

                mMonitorProcess->start(NC_COMMAND, { "monitor" });

                //! Get device enable state
                getDevicePowerState();
                return;
            }
        }

        NC_CRITICAL << "No wifi device is found. Wifi will not function";
    };
    connect(mMonitorProcess, &QProcess::finished, this, startMonitor,Qt::SingleShotConnection);

    //! Get wifi device name
    mMonitorProcess->start(NC_COMMAND, {
                                           NC_ARG_GET_VALUES,
                                           "GENERAL.TYPE,GENERAL.DEVICE",
                                           NC_ARG_DEVICE,
                                           NC_ARG_SHOW,
                                       });
}

NmcliObserver::~NmcliObserver()
{
    if (mMonitorProcess && (mMonitorProcess->state() == QProcess::Starting
                            || mMonitorProcess->state() == QProcess::Running)) {
        mMonitorProcess->kill();
    }
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
    QProcess* process = new QProcess(this);

    //! Check wifi device on/off state
    connect(process, &QProcess::finished, this, [this, process](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) {
                setDevicePowerState(process->readLine().contains("enabled"));
            }
            process->deleteLater();
        }, Qt::SingleShotConnection);

    process->start(NC_COMMAND, {
                                   NC_ARG_GET_VALUES,
                                   "WIFI",
                                   NC_ARG_GENERAL,
                                   "status",
                               });
}

void NmcliObserver::onMonitorProcessReadReady()
{
    QString message = mMonitorProcess->readLine();
    message.chop(1); //! Discard \n
    while(!message.isEmpty()) {
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

        message = mMonitorProcess->readLine();
        message.chop(1); //! Discard \n
    }
}

#include "NmcliInterface.h"

#include "Nmcli.h"

//! Methods implementations

NmcliInterface::NmcliInterface(QObject* parent)
    : QObject { parent }
    , mNmcliObserver { new NmcliObserver(this) }
    , mProcess { new QProcess(this) }
{
    mProcess->setReadChannel(QProcess::StandardOutput);
    connect(mProcess, &QProcess::stateChanged, this, &NmcliInterface::isRunningChanged);
    connect(mProcess, &QProcess::finished, this, [&](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode != 0) {
            emit errorOccured(NmcliInterface::Error(exitCode));
        }
    });

    setupObserver();
}

NmcliInterface::~NmcliInterface()
{
    if (isRunning()) {
        mProcess->kill();
    }
}

bool NmcliInterface::isRunning() const
{
    return mProcess && (mProcess->state() == QProcess::Starting
                        || mProcess->state() == QProcess::Running);
}

WifisList& NmcliInterface::getWifis()
{
    return mWifis;
}

void NmcliInterface::refreshWifis(bool rescan)
{
    if (!mProcess || isRunning()) {
        return;
    }

    connect(mProcess, &QProcess::finished, this, &NmcliInterface::onWifiListRefreshFinished,
            Qt::SingleShotConnection);

    const QStringList args = NC_REFERESH_ARGS + NC_PRINT_MODE_ARGS + QStringList({
                                 NC_ARG_DEVICE,
                                 NC_ARG_WIFI,
                                 NC_ARG_LIST,
                                 NC_ARG_RESCAN,
                                 rescan ? "yes" : "auto"
                             });

    mProcess->start(NC_COMMAND, args);
}

bool NmcliInterface::hasWifiProfile(const WifiInfo* wifi)
{
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.start(NC_COMMAND, {
                                  NC_ARG_GET_VALUES,
                                  "802-11-wireless.seen-bssids",
                                  "--escape",
                                  "no",
                                  NC_ARG_CONNECTION,
                                  NC_ARG_SHOW,
                                  wifi->ssid()
                              });
    waitLoop(&process, NC_WAIT_MSEC);
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

void NmcliInterface::connectToWifi(WifiInfo* wifi, const QString& password)
{
    if (!mProcess || isRunning()) {
        return;
    }

    //! First check if connection is saved
    if (hasWifiProfile(wifi)) {
        connectSavedWifi(wifi, password);
    } else {
        //! Perform connection command
        const QStringList args({
            NC_ARG_DEVICE,
            NC_ARG_WIFI,
            NC_ARG_CONNECT,
            wifi->ssid(),
            NC_ARG_PASSWORD,
            password,
        });

        mProcess->start(NC_COMMAND, args);
    }
}

bool NmcliInterface::connectSavedWifi(WifiInfo* wifi, const QString& password)
{
    //! It's supposed that this private method is only called on a saved wifi, so no need to check
    if (!wifi) {
        return false;
    }

    if (password.isEmpty()) {
        //! Perform connection command
        const QStringList args({
            NC_ARG_CONNECTION,
            NC_ARG_UP,
            wifi->ssid(),
        });

        mProcess->start(NC_COMMAND, args);
    } else {
        //! Modify its password then connect as saved
        connect(mProcess, &QProcess::finished, this,
            [&, wifi](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                    //! Perform connection command
                    const QStringList args({
                        NC_ARG_CONNECTION,
                        NC_ARG_UP,
                        wifi->ssid(),
                    });

                    mProcess->start(NC_COMMAND, args);
                }
            }, Qt::SingleShotConnection);

        const QStringList args({
            NC_ARG_CONNECTION,
            "modify",
            wifi->ssid(),
            "802-11-wireless-security.psk",
            password
        });

        mProcess->start(NC_COMMAND, args);
    }
    return true;
}

void NmcliInterface::disconnectFromWifi(WifiInfo* wifi)
{
    if (!mProcess || isRunning()) {
        return;
    }

    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DOWN,
        wifi->ssid(),
    });

    mProcess->start(NC_COMMAND, args);
}

WifiInfo* NmcliInterface::connectedWifi()
{
    return mConnectedWifi;
}

void NmcliInterface::forgetWifi(WifiInfo* wifi)
{
    if (!mProcess || isRunning()) {
        return;
    }

    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DELETE,
        wifi->ssid(),
    });

    mProcess->start(NC_COMMAND, args);
}

void NmcliInterface::turnWifiDeviceOn()
{
    if (!mProcess || isRunning()) {
        return;
    }

    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "on"
    });

    mProcess->start(NC_COMMAND, args);
}

void NmcliInterface::turnWifiDeviceOff()
{
    if (!mProcess || isRunning()) {
        return;
    }

    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "off"
    });

    mProcess->start(NC_COMMAND, args);
}

void NmcliInterface::addConnection(const QString& name,
                                   const QString& ssid,
                                   const QString& ip4,
                                   const QString& gw4,
                                   const QString& dns,
                                   const QString& security,
                                   const QString& password)
{
    if (!mProcess || isRunning()) {
        return;
    }

    //! Perform connection command
    QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_CON_ADD,
        "type",
        NC_ARG_WIFI,
        "con-name",
        name,
        "wifi.ssid",
        ssid,
        "ip4",
        ip4,
        "gw4",
        gw4,
    });


    if (!security.isEmpty()) {
        args += {
            "802-11-wireless-security.key-mgmt",
            security,
            "802-11-wireless-security.psk",
            password
        };
    }

    if (!dns.isEmpty()) {
        args += {
            "ipv4.dns-options",
            "0",
            "ipv4.ignore-auto-dns",
            "yes"
        };
    }

    //! Connect to this new connection if successfully added
    connect(mProcess, &QProcess::finished, this,
        [&, name, ssid, password, security](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                mWifis.push_back(new WifiInfo(false, ssid, "", 100, security));
                emit wifisChanged();

                mProcess->start(NC_COMMAND, {
                                                NC_ARG_CONNECTION,
                                                NC_ARG_UP,
                                                name
                                            });
            }
        }, Qt::SingleShotConnection);

    mProcess->start(NC_COMMAND, args);
}

QString NmcliInterface::getConnectedWifiBssid() const
{
    //! if mProcess is running use another one. If mProcess is not running use it instead of
    //! creating a new one.
    QProcess* pr;
    if (isRunning()) {
        pr = new QProcess();
        pr->setReadChannel(QProcess::StandardOutput);
    } else {
        pr = mProcess;
    }

    //! nmcli -e no -t -f active,bssid device wifi
    pr->start(NC_COMMAND, {
                              "-t",
                              "-e",
                              "no",
                              NC_ARG_GET_VALUES,
                              "active,bssid",
                              NC_ARG_DEVICE,
                              NC_ARG_WIFI,
                          });

    waitLoop(pr, NC_WAIT_MSEC);
    if (pr->exitStatus() == QProcess::NormalExit && !pr->exitCode()) {
        QByteArray line = pr->readLine();
        while (!line.isEmpty()) {
            if (line.startsWith("yes:")) {
                QString bssid = line.sliced(QString("yes:").length());
                bssid.chop(1);
                return bssid;
            }
            line = pr->readLine();
        }
    } else {
        qDebug() << "nmcli: pr error: " << pr->errorString();
    }

    if (pr != mProcess) {
        pr->deleteLater();
    }

    return "";
}

int NmcliInterface::waitLoop(QProcess* process, int timeout) const
{
    QEventLoop loop;
    // connect signal for handling stopWork
    connect(process, &QProcess::finished, &loop, [&loop]() {
        loop.exit();
    });

    if (timeout == 0) {
        return 0;
    } else if (timeout > 0) {
        // quit will exit with, same as exit(ChangeType::CurrentTemperature)
        QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
    }

    return loop.exec();
}

void NmcliInterface::onGetWifiDeviceNameFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0) {
        QByteArray line = mProcess->readLine();
        while (!line.isEmpty() && line != "wifi\n") {
            line = mProcess->readLine();
        }

        mWifiDevice = mProcess->readLine();
        if (!mWifiDevice.isEmpty()) {
            mWifiDevice.remove(mWifiDevice.size() - 1, 1);
        }
    }
}

void NmcliInterface::onWifiListRefreshFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0) {
        //! First backup current wifis intor another list
        WifisList wifisBackup = mWifis;
        mWifis.clear();

        //! nmcli results are each on one line.do
        QString line = mProcess->readLine(); //! Holds IN-USE of first wifi info in any
        line.remove(line.length() - 1, 1); //! Remove '\n'

        while (!line.isEmpty()) {
            WifiInfo parsedWi;

            //! line is like : IN-USE:* (or no *)
            const int inUseLen = 7; //! Length of 'IN-USE:' string
            parsedWi.setConnected(line.size() > inUseLen ? (line.sliced(inUseLen) == "*") : false );

            //! Read BSSID line: it's in this form: BSSID:<bssid>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int bssidLen = 6; //! Plus one for :
            parsedWi.setBssid(line.size() > bssidLen ? line.sliced(bssidLen) : "");

            //! Read SSID line: it's in this form: SSID:<ssid>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int ssidLen = 5;
            parsedWi.setSsid(line.size() > ssidLen ? line.sliced(ssidLen) : "");

            //! Read SIGNAL line: it's in this form: SIGNAL:<signal>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int signalLen = 7;
            parsedWi.setStrength(line.size() > signalLen ? line.sliced(signalLen).toInt() : 0);

            //! Read SECURITY line: it's in this form: SECURITY:<security>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int securityLen = 9;
            parsedWi.setSecurity(line.size() > securityLen ? line.sliced(securityLen) : "");

            //! Either create a new WifiInfo or update an existing one
            int indexInBackup = -1;
            for (int i = 0; i < wifisBackup.length(); ++i) {
                if (wifisBackup[i]->bssid() == parsedWi.bssid()) {
                    indexInBackup = i;
                    break;
                }
            }

            WifiInfo* wifi = nullptr;
            if (indexInBackup > -1) {
                //! First remove it from wifisBackup
                wifi = wifisBackup.takeAt(indexInBackup);

                wifi->setConnected(parsedWi.connected());
                wifi->setSsid(parsedWi.ssid());
                wifi->setBssid(parsedWi.bssid());
                wifi->setStrength(parsedWi.strength());
                wifi->setSecurity(parsedWi.security());
            } else {
                wifi = new WifiInfo(parsedWi.connected(),
                                    parsedWi.ssid(),
                                    parsedWi.bssid(),
                                    parsedWi.strength(),
                                    parsedWi.security());
            }
            mWifis.push_back(wifi);

            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
        }

        //! Delete all the WifiInfo* in wifisBackup
        for (WifiInfo* wi: wifisBackup) {
            wi->deleteLater();
        }

        emit wifisChanged();
    } else {
        emit errorOccured(NmcliInterface::Error(exitCode));
    }
}

void NmcliInterface::setupObserver()
{
    connect(mNmcliObserver, &NmcliObserver::wifiConnected, this, &NmcliInterface::onWifiConnected);
    connect(mNmcliObserver, &NmcliObserver::wifiDisconnected, this,
            &NmcliInterface::onWifiDisconnected);
    connect(mNmcliObserver, &NmcliObserver::wifiDevicePowerChanged, this, [&]() {
        emit deviceIsOnChanged();
        if (mNmcliObserver->isWifiOn()) {
            refreshWifis();
        }
    });

    connect(mNmcliObserver, &NmcliObserver::wifiNeedAuthentication, this,
            &NmcliInterface::wifiNeedAuthentication);


    connect(mNmcliObserver, &NmcliObserver::wifiForgotten, this, [this](const QString& ssid) {
        if (mConnectedWifi && mConnectedWifi->ssid() == ssid) {
            setConnectedWifi(nullptr);
        }
    });

    connect(mNmcliObserver, &NmcliObserver::wifiIsConnecting, this, [this](const QString ssid) {
        //! Search for a wifi with this bssid.
        for (WifiInfo* wifi : mWifis) {
            if (wifi->ssid() == ssid) {
                wifi->setIsConnecting(true);
                return;
            }
        }
    });
}

void NmcliInterface::setConnectedWifi(WifiInfo* wifiInfo)
{
    if (mConnectedWifi == wifiInfo) {
        return;
    }

    if (mConnectedWifi) {
        mConnectedWifi->setConnected(false);
    }

    mConnectedWifi = wifiInfo;
    if (mConnectedWifi) {
        mConnectedWifi->setConnected(true);
    }

    emit connectedWifiChanged();
}

void NmcliInterface::onWifiConnected(const QString& ssid)
{
    for (WifiInfo* wifi : mWifis) {
        if (wifi->ssid() == ssid) {
            wifi->setIsConnecting(false);
            setConnectedWifi(wifi);

            return;
        }
    }
}

void NmcliInterface::onWifiDisconnected()
{
    if (mConnectedWifi) {
        setConnectedWifi(nullptr);
    }

    //! Also set isConnecting to false if its true in any WifiInfo
    for (auto wifi : mWifis) {
        if (wifi->isConnecting()) {
            wifi->setIsConnecting(false);
        }
    }
}

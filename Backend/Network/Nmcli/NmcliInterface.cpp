#include "NmcliInterface.h"
#include "Nmcli.h"

#include <QDir>
#include <QRegularExpression>

//! Methods implementations

NmcliInterface::NmcliInterface(QObject* parent)
    : QObject { parent }
    , mNmcliObserver { new NmcliObserver(this) }
    , mRefreshProcess { new QProcess(this) }
    , mWifiProcess { new QProcess(this) }
    , mConnectedWifi { nullptr }
{
    mRefreshProcess->setReadChannel(QProcess::StandardOutput);
    mWifiProcess->setReadChannel(QProcess::StandardOutput);

    connect(mWifiProcess, &QProcess::stateChanged, this, &NmcliInterface::busyChanged);
    connect(mRefreshProcess, &QProcess::finished, this, [&](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode != 0) {
            emit errorOccured(NmcliInterface::Error(exitCode));
        }
    });
    connect(mWifiProcess, &QProcess::finished, this, [&](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode != 0) {
            emit errorOccured(NmcliInterface::Error(exitCode));
        }
    });

    setupObserver();
}

NmcliInterface::~NmcliInterface()
{
    if (busyRefreshing()) {
        mRefreshProcess->kill();
    }
    if (busy()) {
        mWifiProcess->kill();
    }
}

bool NmcliInterface::busyRefreshing() const
{
    return mBusyRefreshing;
}

void NmcliInterface::setBusyRefreshing(bool busy)
{
    if (mBusyRefreshing == busy) {
        return;
    }

    mBusyRefreshing = busy;
    emit busyRefreshingChanged();
}

bool NmcliInterface::busy() const
{
    return mWifiProcess && (mWifiProcess->state() == QProcess::Starting
                               || mWifiProcess->state() == QProcess::Running);
}

WifisList& NmcliInterface::getWifis()
{
    return mWifis;
}

void NmcliInterface::refreshWifis(bool rescan)
{
    if (!mRefreshProcess || busyRefreshing()) {
        return;
    }

    mRescanInRefresh = rescan;
    setBusyRefreshing(true);

    //! First update bssid to correct ssid map using iw
    connect(mRefreshProcess, &QProcess::finished, this,
            &NmcliInterface::parseBssidToCorrectSsidMap, Qt::SingleShotConnection);

    mRefreshProcess->start("iw", { "dev", mNmcliObserver->wifiDevice(), "scan" });
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
    if (!mWifiProcess || busy()) {
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
            wifi->bssid(),
            NC_ARG_PASSWORD,
            password,
        });

        mWifiProcess->start(NC_COMMAND, args);
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

        mWifiProcess->start(NC_COMMAND, args);
    } else {
        //! Modify its password then connect as saved
        connect(mWifiProcess, &QProcess::finished, this,
            [&, wifi](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                    //! Perform connection command
                    const QStringList args({
                        NC_ARG_CONNECTION,
                        NC_ARG_UP,
                        wifi->ssid(),
                    });

                    mWifiProcess->start(NC_COMMAND, args);
                }
            }, Qt::SingleShotConnection);

        const QStringList args({
            NC_ARG_CONNECTION,
            "modify",
            wifi->ssid(),
            "802-11-wireless-security.psk",
            password
        });

        mWifiProcess->start(NC_COMMAND, args);
    }
    return true;
}

void NmcliInterface::disconnectFromWifi(WifiInfo* wifi)
{
    if (!mWifiProcess || busy()) {
        return;
    }

    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DOWN,
        wifi->ssid(),
    });

    mWifiProcess->start(NC_COMMAND, args);
}

WifiInfo* NmcliInterface::connectedWifi()
{
    return mConnectedWifi;
}

void NmcliInterface::forgetWifi(WifiInfo* wifi)
{
    if (!mWifiProcess || busy()) {
        return;
    }

    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DELETE,
        wifi->ssid(),
    });

    mWifiProcess->start(NC_COMMAND, args);
}

void NmcliInterface::turnWifiDeviceOn()
{
    if (!mWifiProcess || busy()) {
        return;
    }

    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "on"
    });

    mWifiProcess->start(NC_COMMAND, args);
}

void NmcliInterface::turnWifiDeviceOff()
{
    if (!mWifiProcess || busy()) {
        return;
    }

    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "off"
    });

    mWifiProcess->start(NC_COMMAND, args);
}

void NmcliInterface::addConnection(const QString& name,
                                   const QString& ssid,
                                   const QString& ip4,
                                   const QString& gw4,
                                   const QString& dns,
                                   const QString& security,
                                   const QString& password)
{
    if (!mWifiProcess || busy()) {
        return;
    }

    //! Perform connection command
    QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_CON_ADD,
        "ifname",
        mNmcliObserver->wifiDevice(),
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
    connect(mWifiProcess, &QProcess::finished, this,
        [&, name, ssid, password, security](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                WifiInfo* newWifi = new WifiInfo(false, false, ssid, "", 100, security);
                newWifi->setIsConnecting(true);
                mWifis.push_back(newWifi);

                emit wifisChanged();

                mWifiProcess->start(NC_COMMAND, {
                                                NC_ARG_CONNECTION,
                                                NC_ARG_UP,
                                                name
                                            });
            }
        }, Qt::SingleShotConnection);

    mWifiProcess->start(NC_COMMAND, args);
}

QString NmcliInterface::getConnectedWifiBssid() const
{
    //! if mWifiProcess is running use another one. If mWifiProcess is not running use it instead of
    //! creating a new one.
    QProcess* pr;
    if (busy()) {
        pr = new QProcess();
        pr->setReadChannel(QProcess::StandardOutput);
    } else {
        pr = mWifiProcess;
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

    if (pr != mWifiProcess) {
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

void NmcliInterface::onWifiListRefreshFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0) {
        //! Holds currently connected wifi
        WifiInfo* currentWifi = nullptr;

        //! First backup current wifis intor another list
        WifisList wifisBackup = mWifis;
        mWifis.clear();

        //! nmcli results are each on one line.do
        QString line = mRefreshProcess->readLine(); //! Holds IN-USE of first wifi info in any
        line.remove(line.length() - 1, 1); //! Remove '\n'

        while (!line.isEmpty()) {
            WifiInfo parsedWi;

            //! line is like : IN-USE:* (or no *)
            const int inUseLen = 7; //! Length of 'IN-USE:' string
            parsedWi.setConnected(line.size() > inUseLen ? (line.sliced(inUseLen) == "*") : false );

            //! Read BSSID line: it's in this form: BSSID:<bssid>
            line = mRefreshProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int bssidLen = 6; //! Plus one for :
            parsedWi.setBssid(line.size() > bssidLen ? line.sliced(bssidLen) : "");

            //! Read SSID line: it's in this form: SSID:<ssid>
            line = mRefreshProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int ssidLen = 5;
            parsedWi.setSsid(line.size() > ssidLen ? line.sliced(ssidLen) : "");

            //! If this BSSID is in mBssToCorrectSsidMap it means that the ssid should be corrected
            if (mBssToCorrectSsidMap.contains(parsedWi.bssid()) && !parsedWi.ssid().isEmpty()) {
                //! Store incorrect ssid as it is usefull for data returned from NmcliObserver
                parsedWi.setIncorrectSsid(parsedWi.ssid());
                parsedWi.setSsid(mBssToCorrectSsidMap[parsedWi.bssid()]);
            }

            //! Read SIGNAL line: it's in this form: SIGNAL:<signal>
            line = mRefreshProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int signalLen = 7;
            parsedWi.setStrength(line.size() > signalLen ? line.sliced(signalLen).toInt() : 0);

            //! Read SECURITY line: it's in this form: SECURITY:<security>
            line = mRefreshProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int securityLen = 9;
            parsedWi.setSecurity(line.size() > securityLen ? line.sliced(securityLen) : "");

            //! Skip it if there is a wifi with this bssid in the list or if ssid is an empty string
            auto wiInstance = std::find_if(mWifis.begin(), mWifis.end(), [&parsedWi](WifiInfo* wi) {
                return wi->bssid() == parsedWi.bssid();
            });

            //! Check if wifi is saved
            auto conProfileIter = std::find_if(mConProfiles.cbegin(),
                                               mConProfiles.cend(),
                                               [&parsedWi](const ConnectionProfile& cp) {
                                                   return cp.seenBssids.contains(parsedWi.bssid());
                                               });

            if (conProfileIter != mConProfiles.end()) {
                //! This wifi is saved.
                parsedWi.setIsSaved(true);

                if (parsedWi.ssid().isEmpty()) {
                    //! This is probably a hidden network. Check if saved.
                    parsedWi.setSsid(conProfileIter->ssid);
                }

                //! Remove conProfileIter from mConProfiles
                mConProfiles.erase(conProfileIter);
            }

            if (wiInstance == mWifis.end() && !parsedWi.ssid().isEmpty()) {
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
                    wifi->setIsSaved(parsedWi.isSaved());
                    wifi->setIncorrectSsid(parsedWi.incorrectSsid());
                    wifi->setSsid(parsedWi.ssid());
                    wifi->setBssid(parsedWi.bssid());
                    wifi->setStrength(parsedWi.strength());
                    wifi->setSecurity(parsedWi.security());
                } else {
                    wifi = new WifiInfo(parsedWi.connected(),
                                        parsedWi.isSaved(),
                                        parsedWi.ssid(),
                                        parsedWi.bssid(),
                                        parsedWi.strength(),
                                        parsedWi.security()
                                        );
                    wifi->setIncorrectSsid(parsedWi.incorrectSsid());
                }
                mWifis.push_back(wifi);

                if (wifi->connected()) {
                    currentWifi = wifi;
                }
            }

            line = mRefreshProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
        }

        setConnectedWifi(currentWifi);

        //! Delete all the WifiInfo* in wifisBackup
        for (WifiInfo* wi: wifisBackup) {
            wi->deleteLater();
        }

        setBusyRefreshing(false);

        //! Add all other connection profiles that are not already added in the list of wifis
        for (const auto& p : mConProfiles) {
            if (p.seenBssids == "") {
                //! This is probabley added after an incorrect password, do not add it
                continue;
            }
            mWifis.push_back(
                new WifiInfo(false,
                             true, //! is saved
                             p.ssid,
                             "",
                             -1, //! Strength=-1 so they are distinguishable from in-range wifis
                             "",
                             this
                             ));
        }
        mConProfiles.clear();

        emit wifisChanged();
    } else {
        setBusyRefreshing(false);
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

    connect(mNmcliObserver, &NmcliObserver::wifiNeedAuthentication, this, [&](const QString& ssid) {
        if (ssid.isEmpty()) return;

        for (WifiInfo* wifi : mWifis) {
            if (wifi->ssid() == ssid || wifi->incorrectSsid() == ssid) {
                emit wifiNeedAuthentication(wifi);
                return;
            }
        }
    });

    connect(mNmcliObserver, &NmcliObserver::wifiForgotten, this, [this](const QString& ssid) {
        if (ssid.isEmpty()) {
            return;
        }

        auto forgottenWifi = std::find_if(mWifis.cbegin(), mWifis.cend(), [&](WifiInfo* w) {
            return w->ssid() == ssid || w->incorrectSsid() == ssid;
        });
        if (forgottenWifi != mWifis.end()) {
            (*forgottenWifi)->setIsSaved(false);

            if ((*forgottenWifi)->strength() < 0) { //! NOTE: Convention for saved non-in-range wifi
                mWifis.erase(forgottenWifi);
                emit wifisChanged();
            }
        }

        if (mConnectedWifi && (mConnectedWifi->ssid() == ssid
                               || mConnectedWifi->incorrectSsid() == ssid)) {
            mConnectedWifi->setIsSaved(false);
            setConnectedWifi(nullptr);
        }
    });

    connect(mNmcliObserver, &NmcliObserver::wifiIsConnecting, this, [this](const QString ssid) {
        //! Search for a wifi with this bssid.
        for (WifiInfo* wifi : mWifis) {
            if (wifi->ssid() == ssid || wifi->incorrectSsid() == ssid) {
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
        if (wifi->ssid() == ssid || wifi->incorrectSsid() == ssid) {
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

void NmcliInterface::parseBssidToCorrectSsidMap(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode != 0 || exitStatus != QProcess::NormalExit) {
        setBusyRefreshing(false);
        emit errorOccured(NmcliInterface::Error::IWFetchError);

        return;
    }

    //! Clear the map first
    mBssToCorrectSsidMap.clear();

    QString iwOutStr = mRefreshProcess->readAllStandardOutput();

    //! Regular expression to find BSS and SSID
    static QRegularExpression bssSsidRegex(R"((BSS\s([a-zA-Z0-9]{2}\:)+[a-zA-Z0-9]{2})[\s\S]*?(?=SSID\:)(SSID\:\s.*))");

    QRegularExpressionMatchIterator bssMatch = bssSsidRegex.globalMatch(iwOutStr);

    //! Regular expression to match the hexadecimal representation
    static QRegularExpression unknownCharsRegex("\\\\x([0-9A-Fa-f]{2})");

    while (bssMatch.hasNext()) {
        QRegularExpressionMatch mch = bssMatch.next();

        QString bssid = mch.captured(1);
        QString ssid = mch.captured(3);

        //! Find if this ssid contains hex numbers (it probabely contains unknown chars)
        QRegularExpressionMatchIterator matchIterator = unknownCharsRegex.globalMatch(ssid);

        if (matchIterator.hasNext()) {
            // find and append all consecutive matches
            std::map<QString, QString> matchStrs;
            QString matchStr;
            QByteArray byteArray;
            int lastCaptureEnd = -1;
            int numberOfHexPair = 0; //! For unsopported characters hex value has 3 pair of numbers

            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                // reset and search for next consecutive
                if (lastCaptureEnd != -1 && (match.capturedStart() > lastCaptureEnd || numberOfHexPair == 3)) {
                    numberOfHexPair = 0;

                    // insert current match
                    if (!matchStr.isEmpty()) {
                        if (matchStrs.count(matchStr) == 0) {
                            // insert the equivalent
                            matchStrs.insert({matchStr, QString::fromUtf8(byteArray)});
                        }
                    }

                    matchStr = {};
                    byteArray = {};
                }

                numberOfHexPair++;
                lastCaptureEnd = match.capturedEnd();
                matchStr.append(match.captured());
                QString hexString = match.captured(1); // Extract the hexadecimal characters
                byteArray.append(static_cast<char>(hexString.toInt(nullptr, 16))); // Convert hexadecimal to integer and then to char
            }

            // insert last match
            if (!matchStr.isEmpty()) {
                if (matchStrs.count(matchStr) == 0) {
                    // insert the equivalent
                    matchStrs.insert({matchStr, QString::fromUtf8(byteArray)});
                }
            }

            for (const QPair<QString, QString>& currMatch : matchStrs) {
                ssid.replace(currMatch.first, currMatch.second);
            }

            ssid = ssid.sliced(6);
            bssid = bssid.sliced(4);

            mBssToCorrectSsidMap[bssid.toUpper()] = ssid;
        }
    }

    //! Get the list of wifi connections
    updateSavedWifis();
}

void NmcliInterface::updateSavedWifis()
{
    mConProfiles.clear();

    //! First get the list of all saved connections
    const QStringList args = QStringList {
        NC_ARG_GET_VALUES,
        "NAME",
        "-m",
        "multiline",
        "--escape",
        "no",
        NC_ARG_CONNECTION,
    };
    mRefreshProcess->start(NC_COMMAND, args);

    waitLoop(mRefreshProcess, 500);

    QString line = mRefreshProcess->readLine(); //! Holds IN-USE of first wifi info in any

    if (mRefreshProcess->exitCode() == 0) {
        QProcess conProcess;

        while (!line.isEmpty()) {
            QString connectionName = line.sliced(5, line.length() - 5 - 1);

            if (mBssToCorrectSsidMap.contains(connectionName)) {
                connectionName = mBssToCorrectSsidMap[connectionName];
            }

            //! Get profile info of this connection
            conProcess.start(NC_COMMAND, {
                                                   "--get-values",
                                                   "802-11-wireless.ssid,802-11-wireless.seen-bssids",
                                                   "--escape",
                                                   "no",
                                                   NC_ARG_CONNECTION,
                                                   NC_ARG_SHOW,
                                                   connectionName,
                                               });
            waitLoop(&conProcess, 500);

            if (conProcess.exitCode() == 0) {
                QString ssid = conProcess.readLine();
                ssid.remove(ssid.length() - 1, 1); //! Remove '\n'

                QString seenBssids = conProcess.readLine();
                seenBssids.remove(seenBssids.length() - 1, 1); //! Remove '\n'

                mConProfiles.emplace_back(ssid, seenBssids);
            }

            line = mRefreshProcess->readLine();
        }
    } else {
        NC_CRITICAL << mRefreshProcess->readAll();
    }

    //! Now refresh wifis
    doRefreshWifi();
}

void NmcliInterface::doRefreshWifi()
{
    connect(mRefreshProcess, &QProcess::finished, this, &NmcliInterface::onWifiListRefreshFinished,
            Qt::SingleShotConnection);

    const QStringList args = NC_REFERESH_ARGS + NC_PRINT_MODE_ARGS + QStringList({
                                 NC_ARG_DEVICE,
                                 NC_ARG_WIFI,
                                 NC_ARG_LIST,
                                 NC_ARG_RESCAN,
                                 mRescanInRefresh ? "yes" : "auto"
                             });

    mRefreshProcess->start(NC_COMMAND, args);
}

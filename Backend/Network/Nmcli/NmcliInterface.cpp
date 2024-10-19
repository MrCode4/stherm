#include "NmcliInterface.h"
#include "Nmcli.h"

#include <QDir>
#include <QRegularExpression>

//! Methods implementations

NmcliInterface::NmcliInterface(QObject* parent)
    : QObject { parent }
    , mNmcliObserver { new NmcliObserver(this) }
    , mConnectedWifi { nullptr }
    , mConProfilesWatcher { new QFileSystemWatcher(this) }
    , mCliCommon(new NmCli(this))
    , mCliRefresh(new NmCli(this))
    , mCliWifi(new NmCli(this))
    , mCliProfiles(new NmCli(this))
{
    connect(mCliRefresh, &ProcessExecutor::finished, this, [&](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode != 0) {
            emit errorOccured(NmcliInterface::Error(exitCode));
        }
    });
    connect(mCliWifi, &ProcessExecutor::finished, this, [&](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::NormalExit && exitCode != 0) {
            emit errorOccured(NmcliInterface::Error(exitCode));
        }
    });

    setupObserver();

    initializeConProfilesWatcher();
    scanConProfiles(); //! It uses an async procedure
}

NmcliInterface::~NmcliInterface()
{
    if (busyRefreshing()) {
        mCliRefresh->kill();
    }
    if (busy()) {
        mCliWifi->kill();
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
    return mBusy;
}

void NmcliInterface::setBusy(bool busy)
{
    if (mBusy == busy) {
        return;
    }

    mBusy = busy;
    emit busyChanged();
}

WifisList& NmcliInterface::getWifis()
{
    return mWifis;
}

void NmcliInterface::refreshWifis(bool rescan)
{
    if  (busyRefreshing() || busy()) {
        return;
    }

    mRescanInRefresh = rescan;
    setBusyRefreshing(true);

    //! First update bssid to correct ssid map using iw
    mCliRefresh->execAsync("iw", { "dev", mNmcliObserver->wifiDevice(), "scan" }, [this] (QProcess* process) {
        parseBssidToCorrectSsidMap(process);
    });
}

bool NmcliInterface::hasWifiProfile(const WifiInfo* wifi)
{
    return mCliCommon->hasWifiProfile(wifi->ssid());
}

void NmcliInterface::connectToWifi(WifiInfo* wifi, const QString& password)
{
    if (!wifi || wifi->isConnecting() || wifi->connected() || busy()) {
        return;
    }

    //! First check if connection is saved
    if (hasWifiProfile(wifi)) {
        connectSavedWifi(wifi, password);
    } else {
        setBusy(true);                
        mCliWifi->connectToUnsavedWifi(wifi->bssid(), password, [this] (QProcess*) {
            setBusy(false);
        });
    }
}

bool NmcliInterface::connectSavedWifi(WifiInfo* wifi, const QString& password)
{
    //! It's supposed that this private method is only called on a saved wifi, so no need to check
    if (!wifi) {
        return false;
    }

    setBusy(true);

    mCliWifi->connectToSavedWifi(wifi->ssid(), wifi->security(), password, [this] (QProcess*) {
        setBusy(false);
    });

    return true;
}

void NmcliInterface::disconnectFromWifi(WifiInfo* wifi)
{
    if (!wifi || !wifi->connected() || busy()) {
        return;
    }

    setBusy(true);

    //! Perform disconnect command
    mCliWifi->disconnectFromWifi(wifi->ssid(), [this] (QProcess*) {
        setBusy(false);
    });
}

WifiInfo* NmcliInterface::connectedWifi()
{
    return mConnectedWifi;
}

void NmcliInterface::forgetWifi(WifiInfo* wifi)
{
    if (!wifi || !wifi->isSaved() || busy()) {
        return;
    }

    setBusy(true);
    //! Perform disconnect command
    mCliWifi->forgetWifi(wifi->ssid(), [this] (QProcess*) {
        setBusy(false);
    });
}

void NmcliInterface::turnWifiDeviceOn()
{
    if (busy()) {
        return;
    }

    setBusy(true);

    //! Perform connection command
    mCliWifi->turnWifiDeviceOn([this] (QProcess*) {
        setBusy(false);
    });
}

void NmcliInterface::turnWifiDeviceOff()
{
    if (busy()) {
        return;
    }

    setBusy(true);

    //! Perform connection command
    mCliWifi->turnWifiDeviceOff([this] (QProcess*) {
        setBusy(false);
    });
}

void NmcliInterface::addConnection(const QString& name,
                                   const QString& ssid,
                                   const QString& ip4,
                                   const QString& gw4,
                                   const QString& dns,
                                   const QString& security,
                                   const QString& password)
{
    if (busy()) {
        return;
    }

    setBusy(true);

    //! Connect to this new connection if successfully added
    auto onFinished = [this, name, ssid, security] (QProcess* process) {
        if (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) {
            //! Either create a new WifiInfo instance or use an existing one
            auto wifiIter = std::find_if(mWifis.begin(), mWifis.end(), [ssid] (WifiInfo* w) {
                return w->ssid() == ssid;
            });

            WifiInfo* wifi = nullptr;
            if (wifiIter == mWifis.end()) {
                wifi = new WifiInfo(false, true, ssid, "", 100, security);
                mWifis.push_back(wifi);
            } else {
                wifi = *wifiIter;
                wifi->setIsSaved(true);
            }
            wifi->setIsConnecting(true);
            emit wifisChanged();
            mCliWifi->connectToSavedWifi(name, wifi->security(), "", [this] (QProcess*) {
                setBusy(false);
            });
        } else {
            setBusy(false);
        }
    };

    mCliWifi->addConnection(mNmcliObserver->wifiDevice(), name, ssid, ip4, gw4, dns, security, password, onFinished);
}

QString NmcliInterface::getConnectedWifiBssid() const
{
    return mCliWifi->getConnectedWifiBssid();
}

void NmcliInterface::onWifiListRefreshFinished(QProcess* process)
{
    if (process->exitCode() == 0) {
        //! Indexes of the connection profile that are in range and should not be added to the
        //! wifi list anymore
        QMap<QString, bool> alreadyAddedConProfiles;

        //! Holds currently connected wifi
        WifiInfo* currentWifi = nullptr;

        //! First backup current wifis intor another list
        WifisList wifisBackup = mWifis;
        mWifis.clear();

        //! nmcli results are each on one line.do
        QString line = process->readLine(); //! Holds IN-USE of first wifi info in any
        line.remove(line.length() - 1, 1); //! Remove '\n'

        while (!line.isEmpty()) {
            WifiInfo parsedWi;

            //! line is like : IN-USE:* (or no *)
            const int inUseLen = 7; //! Length of 'IN-USE:' string
            parsedWi.setConnected(line.size() > inUseLen ? (line.sliced(inUseLen) == "*") : false );

            //! Read BSSID line: it's in this form: BSSID:<bssid>
            line = process->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int bssidLen = 6; //! Plus one for :
            parsedWi.setBssid(line.size() > bssidLen ? line.sliced(bssidLen) : "");

            //! Read SSID line: it's in this form: SSID:<ssid>
            line = process->readLine();
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
            line = process->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int signalLen = 7;
            parsedWi.setStrength(line.size() > signalLen ? line.sliced(signalLen).toInt() : 0);

            //! Read SECURITY line: it's in this form: SECURITY:<security>
            line = process->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int securityLen = 9;

            QString secrityTypeText = line.size() > securityLen ? line.sliced(securityLen) : "";

            if (mBssToCorrectSecurityMap.contains(parsedWi.bssid())) {
                secrityTypeText = mBssToCorrectSecurityMap[parsedWi.bssid()];
            }

            parsedWi.setSecurity(secrityTypeText);

            //! Skip it if there is a wifi with this bssid in the list or if ssid is an empty string
            auto wiInstance = std::find_if(mWifis.begin(), mWifis.end(), [&parsedWi](WifiInfo* wi) {
                return wi->bssid() == parsedWi.bssid();
            });

            //! Check if wifi is saved
            //! NOTE: When a wifi password is entered incorrectly the connection is added in the
            //! NetworkManager but the seenBssids is empty so only comparing bssid causes this wifi
            //! not to be displayed as saved wifi. This is why the second condition is added.
            //! This might cause bugs in hidden networks.
            ConnectionProfile* conProfileForThis = nullptr;
            for (ConnectionProfile& cp : mConProfiles) {
                if (alreadyAddedConProfiles.contains(cp.ssid)) {
                    //! It's already added to wifis
                    continue;
                }

                if (cp.ssid == parsedWi.ssid()) {
                    alreadyAddedConProfiles[cp.ssid] = true;
                    conProfileForThis = &cp;
                }
            }

            if (conProfileForThis) {
                //! This wifi is saved.
                parsedWi.setIsSaved(true);

                if (parsedWi.ssid().isEmpty()) {
                    //! This is probably a hidden network. Check if saved.
                    parsedWi.setSsid(conProfileForThis->ssid);
                }
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

            line = process->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
        }

        setConnectedWifi(currentWifi);

        //! Delete all the WifiInfo* in wifisBackup
        for (WifiInfo* wi: wifisBackup) {
            wi->deleteLater();
        }

        //! Add all other connection profiles that are not already added in the list of wifis
        for (ConnectionProfile& cp : mConProfiles) {
            if (alreadyAddedConProfiles.contains(cp.ssid)) {
                continue;
            }

            mWifis.push_back(
                new WifiInfo(false,
                             true, //! is saved
                             cp.ssid,
                             "",
                             -1, //! Strength=-1 so they are distinguishable from in-range wifis
                             "",
                             this
                             ));
        }

        setBusyRefreshing(false);

        emit wifisChanged();
    } else {
        setBusyRefreshing(false);
        emit errorOccured(NmcliInterface::Error(process->exitCode()));
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
            if ((wifi->ssid() == ssid || wifi->incorrectSsid() == ssid) && !wifi->connected()) {
                //! This wifi is now saved if it wasn't previously
                if (!wifi->isSaved()) {
                    wifi->setIsSaved(true);
                }
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
    for (int i = 0; i < mWifis.length(); ++i) {
        WifiInfo* wifi = mWifis[i];
        if (wifi->ssid() == ssid || wifi->incorrectSsid() == ssid) {
            wifi->setIsConnecting(false);
            setConnectedWifi(wifi);

            //! Also remove it from the list of mWifis
            mWifis.remove(i);

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

void NmcliInterface::parseBssidToCorrectSsidMap(QProcess* process)
{
    if (process->exitCode() != 0 || process->exitStatus() != QProcess::NormalExit) {
        emit errorOccured(NmcliInterface::Error::IWFetchError);

        NC_DEBUG << "Error iw scan, aborting refresh";
        setBusyRefreshing(false);
        return;
    }

    //! Clear the map first
    mBssToCorrectSsidMap.clear();

    QString iwOutStr = process->readAllStandardOutput();

    // Regular expression to match and extract Wi-Fi details:
    // 1. BSS MAC address in the format xx:xx:xx:xx:xx:xx
    // 2. SSID (network name), which can contain any characters except newline or carriage return
    // 3. Authentication suites, which may include various authentication methods
    static QRegularExpression wifiDetailsRegex(R"(BSS\s([a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2}:[a-fA-F0-9]{2})|SSID:\s([^\n\r]+)|Authentication suites:\s(.+))");

    QRegularExpressionMatchIterator wifiMatchIterator = wifiDetailsRegex.globalMatch(iwOutStr);

    QString bssid{}, ssid{}, auth{};

    enum IterationState { Continue = 0, Finalize, Done };

    IterationState iterationState = IterationState::Continue;

    while (iterationState != IterationState::Done) {
        QRegularExpressionMatch currentMatch = wifiMatchIterator.next();

        bool isBSSIDChanged = (currentMatch.captured(1).isEmpty() == false)
                              && (bssid.isEmpty() == false)
                              && (bssid != currentMatch.captured(1).toUpper());

        // Check if the BSSID has changed or if this is the last iteration.
        // If so, save the current SSID and authentication type to the respective maps if they are valid.
        if (isBSSIDChanged == true || iterationState == IterationState::Finalize) {
            QString replacedHexSSID = decodeHexToChars(ssid);

            bool isSSIDReplaced = (ssid.isEmpty() == false) && (replacedHexSSID != ssid)
                                  && (bssid.isEmpty() == false);
            if (isSSIDReplaced) {
                mBssToCorrectSsidMap[bssid] = replacedHexSSID;
            }

            if ((auth.isEmpty() == false) && (bssid.isEmpty() == false)) {
                mBssToCorrectSecurityMap[bssid] = auth;
            }

            bssid.clear();
            ssid.clear();
            auth.clear();
        }

        // Store new Wi-Fi details if captured by the regex.
        if (currentMatch.captured(1).isEmpty() == false) {
            bssid = currentMatch.captured(1).toUpper();
        } else if (currentMatch.captured(2).isEmpty() == false) {
            ssid = currentMatch.captured(2);
        } else if (currentMatch.captured(3).isEmpty() == false) {
            auth = currentMatch.captured(3);
        }

        // Switch to Finalize when no matches remain, allowing final handling of the last Wi-Fi details.
        if (wifiMatchIterator.hasNext() == false && iterationState == IterationState::Continue) {
            iterationState = IterationState::Finalize;
        }
        else if (iterationState == IterationState::Finalize) {
            iterationState = IterationState::Done;
        }
    }

    //! Get the list of wifi connections
    doRefreshWifi();
}

void NmcliInterface::doRefreshWifi()
{
    if (busyRefreshing()) {
        mCliRefresh->refreshWifi(mRescanInRefresh, [this] (QProcess* process) {
            onWifiListRefreshFinished(process);
        });
    }
}

void NmcliInterface::scanConProfiles()
{
    if (mBusyUpdatingConProfiles) {
        return;
    }
    mBusyUpdatingConProfiles = true;

    mCliProfiles->scanConnectionProfiles([this](QProcess* process) {
        updateConProfilesList(process);
    });
}

void NmcliInterface::updateNextConnectionProfile(QSharedPointer<QTextStream> stream, ProfilesList* profiles)
{
    QString line = stream->readLine();
    if (line.isEmpty()) {
        //! It's over, delete the profiles
        delete profiles;
        mBusyUpdatingConProfiles = false;

        return;
    }

    QString conName = line.sliced(5, line.length() - 5);

    if (mBssToCorrectSsidMap.contains(conName)) {
        conName = mBssToCorrectSsidMap[conName];
    }

    //! If a connection with this name is already in the list of profiles, skip it
    auto wi = std::find_if(profiles->begin(), profiles->end(),
                           [&](ConnectionProfile p) {
                               return conName == p.ssid;
                           });

    if (wi == profiles->end()) {
        //! Get profile info of this connection
        mCliCommon->getProfileInfoByNameAsync(
            conName, [this, stream, profiles] (const QString& ssid, const QString& seenBssids) {
                if (!ssid.isEmpty() || !seenBssids.isEmpty()) {
                    mConProfiles.emplace_back(ssid, seenBssids);
                }
                updateNextConnectionProfile(stream, profiles);
            });
    } else {
        //! The connection already exists, push it to mConProfiles
        mConProfiles.push_back(*wi);
        updateNextConnectionProfile(stream, profiles);
    }
}

void NmcliInterface::updateConProfilesList(QProcess* process)
{
    if (process->exitCode() == 0 && process->exitStatus() == QProcess::NormalExit) {
        //! Copy mConProfiles to aviod requesting for connections that already exists
        //! Note: This pointer must be deleted when done with and updateNextConnectionProfile()
        //! should do this when profiles are finished
        ProfilesList* conProfilesBak = new ProfilesList(mConProfiles);
        mConProfiles.clear();

        QSharedPointer<QTextStream> stream(new QTextStream(process->readAll()));

        updateNextConnectionProfile(stream, conProfilesBak);
    } else {
        // readAll() may have more info than errorString which is printed in parent.
        NC_WARN << process->readAll();
        mBusyUpdatingConProfiles = false;
    }
}

void NmcliInterface::initializeConProfilesWatcher()
{
    if (!mConProfilesWatcher) {
        return;
    }

    mConProfilesWatcher->addPath("/etc/NetworkManager/system-connections");

    connect(mConProfilesWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString& path) {
        scanConProfiles();
    });
}

QString NmcliInterface::decodeHexToChars(const QString &ssid)
{
    QString replacedSSID{ssid};
    //! Regular expression to match the hexadecimal representation
    static QRegularExpression unknownCharsRegex("\\\\x([0-9A-Fa-f]{2})");

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

                matchStr.clear();
                byteArray.clear();
            }

            numberOfHexPair++;
            lastCaptureEnd = match.capturedEnd();
            matchStr.append(match.captured());
            QString hexString = match.captured(1); // Extract the hexadecimal characters
            byteArray.append(static_cast<char>(hexString.toInt(nullptr, 16))); // Convert hexadecimal to integer and then to char
        }

        // insert last match
        if (matchStr.isEmpty() == false) {
            if (matchStrs.count(matchStr) == 0) {
                // insert the equivalent
                matchStrs.insert({matchStr, QString::fromUtf8(byteArray)});
            }
        }

        for (const QPair<QString, QString>& currMatch : matchStrs) {
            replacedSSID.replace(currMatch.first, currMatch.second);
        }
    }

    return replacedSSID;
}

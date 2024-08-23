#include "Nmcli.h"

void NmCli::turnWifiDeviceOn(ExitedCallback callback)
{
    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "on"
    });

    execAsync(NC_COMMAND, args, callback);
}

void NmCli::turnWifiDeviceOff(ExitedCallback callback)
{
    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "off"
    });

    execAsync(NC_COMMAND, args, callback);
}

void NmCli::refreshWifi(bool rescan, ExitedCallback callback)
{
    const QStringList args = NC_REFERESH_ARGS + NC_PRINT_MODE_ARGS + QStringList({
                                 NC_ARG_DEVICE,
                                 NC_ARG_WIFI,
                                 NC_ARG_LIST,
                                 NC_ARG_RESCAN,
                                 rescan ? "yes" : "auto"
                             });

    execAsync(NC_COMMAND, args, callback);
}

void NmCli::scanConnectionProfiles(ExitedCallback callback)
{
    //! Get the list of all saved connections
    const QStringList args = QStringList {
        NC_ARG_GET_VALUES,
        "NAME",
        "-m",
        "multiline",
        "--escape",
        "no",
        NC_ARG_CONNECTION,
    };
    execAsync(NC_COMMAND, args, callback);
}

void NmCli::getDevicePowerState(ExitedCallback callback)
{
    const QStringList args = QStringList {
        NC_ARG_GET_VALUES,
        "WIFI",
        NC_ARG_GENERAL,
        "status",
    };
    execAsync(NC_COMMAND, args, callback);
}

void NmCli::getWifiDeviceName(ExitedCallback callback)
{
    const QStringList args = QStringList {
        NC_ARG_GET_VALUES,
        "GENERAL.TYPE,GENERAL.DEVICE",
        NC_ARG_DEVICE,
        NC_ARG_SHOW,
    };
    execAsync(NC_COMMAND, args, callback);
}

void NmCli::startMonitoring(InitCallback callback)
{
    execAsync(NC_COMMAND, { "monitor" }, nullptr, callback);
}

void NmCli::addConnection(
    const QString& deviceMac,
    const QString& name,
    const QString& ssid,
    const QString& ip4,
    const QString& gw4,
    const QString& dns,
    const QString& security,
    const QString& password,
    ExitedCallback callback)
{
    //! Perform connection command
    QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_ADD,
        "ifname",
        deviceMac,
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

    execAsync(NC_COMMAND, args, callback);
}

void NmCli::getProfileInfoByNameAsync(const QString& connName, std::function<void (const QString&, const QString&)> callback)
{
    //! Get profile info of this connection
    QStringList args = {
        "--get-values",
        "802-11-wireless.ssid,802-11-wireless.seen-bssids",
        "--escape",
        "no",
        NC_ARG_CONNECTION,
        NC_ARG_SHOW,
        connName,
    };

    auto onFinished = [this, connName, callback] (QProcess* process) {
        if (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) {
            QString ssid = process->readLine();
            ssid.remove(ssid.length() - 1, 1); //! Remove '\n'

            // can have only one bssid or be empty or have multiple values separated by comma delimiter
            QString seenBssids = process->readLine();
            seenBssids.remove(seenBssids.length() - 1, 1); //! Remove '\n'

            callback(ssid, seenBssids);
        } else {
            // readAll() may have more info than errorString which is printed in parent.
            NC_WARN << process->exitCode() << process->readAll() << process->readAllStandardError() << connName;

            //! Call the callbck with empty strings.
            callback("", "");
        }
    };

    execAsync(NC_COMMAND, args, onFinished);
}

void NmCli::getProfileInfoByName(const QString& connName, std::function<void (const QString&, const QString&)> callback)
{
    //! Get profile info of this connection
    QStringList args = {
        "--get-values",
        "802-11-wireless.ssid,802-11-wireless.seen-bssids",
        "--escape",
        "no",
        NC_ARG_CONNECTION,
        NC_ARG_SHOW,
        connName,
    };

    auto onFinished = [this, connName, callback] (QProcess* process) {
        if (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) {
            QString ssid = process->readLine();
            ssid.remove(ssid.length() - 1, 1); //! Remove '\n'

            // can have only one bssid or be empty or have multiple values separated by comma delimiter
            QString seenBssids = process->readLine();
            seenBssids.remove(seenBssids.length() - 1, 1); //! Remove '\n'

            callback(ssid, seenBssids);
        } else {
            // readAll() may have more info than errorString which is printed in parent.
            NC_WARN << process->exitCode() << process->readAll() << connName;

            //! Call the callbck with empty strings.
            callback("", "");
        }
    };

    execSync(NC_COMMAND, args, onFinished, 1000);
}

QString NmCli::getConnectedWifiBssid()
{
    QString wifiName = "";
    //! nmcli -e no -t -f active,bssid device wifi
    QStringList arg = {
        "-t",
        "-e",
        "no",
        NC_ARG_GET_VALUES,
        "active,bssid",
        NC_ARG_DEVICE,
        NC_ARG_WIFI,
    };

    auto onFinished = [&wifiName](QProcess* process) {
        if (process->exitStatus() == QProcess::NormalExit && !process->exitCode()) {
            QByteArray line = process->readLine();
            while (!line.isEmpty()) {
                if (line.startsWith("yes:")) {
                    QString bssid = line.sliced(QString("yes:").length());
                    bssid.chop(1);
                    wifiName = bssid;
                    return;
                }
                line = process->readLine();
            }
        } else {
            qDebug() << "nmcli: pr error: " << process->errorString();
        }
    };
    execSync(NC_COMMAND, arg, onFinished, NC_WAIT_MSEC);
    return wifiName;
}

bool NmCli::hasWifiProfile(const QString& ssid)
{
    bool hasProfile = false;
    QStringList arg = {
        NC_ARG_GET_VALUES,
        "802-11-wireless.seen-bssids",
        "--escape",
        "no",
        NC_ARG_CONNECTION,
        NC_ARG_SHOW,
        ssid
    };

    auto onFinished = [&hasProfile](QProcess* process) {
        hasProfile = process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0;
    };

    execSync(NC_COMMAND, arg, onFinished, NC_WAIT_MSEC);
    return hasProfile;
}


void NmCli::connectToUnsavedWifi(const QString& bssid, const QString& password, ExitedCallback callback)
{
    //! Perform connection command
    const QStringList args({
        NC_ARG_DEVICE,
        NC_ARG_WIFI,
        NC_ARG_CONNECT,
        bssid,
        NC_ARG_PASSWORD,
        password,
    });

    execAsync(NC_COMMAND, args, callback);
}

void NmCli::connectToSavedWifi(const QString& ssid, const QString& password, ExitedCallback callback)
{
    if (password.isEmpty()) {
        //! Perform connection command
        const QStringList args({
            NC_ARG_CONNECTION,
            NC_ARG_UP,
            ssid,
        });
        execAsync(NC_COMMAND, args, callback);
    } else {
        //! Modify its password then connect as saved
        auto onFinished = [this, ssid, callback] (QProcess* process) {
            if (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) {
                //! Perform connection command
                const QStringList args({
                    NC_ARG_CONNECTION,
                    NC_ARG_UP,
                    ssid,
                });
                execAsync(NC_COMMAND, args, callback);
            } else {
                callback(process);
            }
        };

        const QStringList args({
            NC_ARG_CONNECTION,
            NC_ARG_MODIFY,
            ssid,
            "802-11-wireless-security.psk",
            password
        });

        execAsync(NC_COMMAND, args, onFinished);
    }
}

void NmCli::disconnectFromWifi(const QString& ssid, ExitedCallback callback)
{
    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DOWN,
        ssid,
    });

    execAsync(NC_COMMAND, args, callback);
}

void NmCli::forgetWifi(const QString& ssid, ExitedCallback callback)
{
    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DELETE,
        ssid,
    });

    execAsync(NC_COMMAND, args, callback);
}

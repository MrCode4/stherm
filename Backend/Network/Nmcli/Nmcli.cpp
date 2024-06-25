#include "Nmcli.h"
#include <QEventLoop>
#include <QTimer>

int Cli::waitLoop(QProcess* process, uint timeout) const
{
    QEventLoop loop;
    // connect signal for handling stopWork
    connect(process, &QProcess::finished, &loop, [&loop]() {
        loop.exit();
    });

    // quit will exit with, same as exit(ChangeType::CurrentTemperature)
    QTimer::singleShot(timeout, &loop, [&loop, process](){
        if (process->state() != QProcess::NotRunning) {
            process->terminate();
        }
    });

    return loop.exec();
}

void Cli::execSync(const QString& command, const QStringList& args, ExitedCallback callback, uint timeout)
{
    if (timeout <= 0) {
        timeout = NC_WAIT_MSEC;
    }

    QProcess process;
    mProcesses.append(&process);
    process.setReadChannel(QProcess::StandardOutput);
    process.start(command, args);
    waitLoop(&process, timeout);
    mProcesses.removeOne(&process);
    if (callback) {
        callback(&process);
    }
    emit finished(process.exitCode(), process.exitStatus());
}

void Cli::execAsync(const QString& command, const QStringList& args, ExitedCallback callback)
{
    QProcess* process = new QProcess;
    mProcesses.append(process);
    process->setReadChannel(QProcess::StandardOutput);
    process->start(command, args);
    connect(process, &QProcess::finished, this, [this, process, callback](int exitCode, QProcess::ExitStatus exitStatus) {
        mProcesses.removeOne(process);
        if (callback) {
            callback(process);
        }
        emit finished(process->exitCode(), process->exitStatus());
        delete process;
    });
}

void Cli::kill()
{
    while (!mProcesses.isEmpty()) {
        mProcesses.takeFirst()->kill();
    }
}

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
        NC_ARG_CON_ADD,
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

    auto onFinished = [&] (QProcess* process) {
        if (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) {
            QString ssid = process->readLine();
            ssid.remove(ssid.length() - 1, 1); //! Remove '\n'

            // can have only one bssid or be empty or have multiple values separated by comma delimiter
            QString seenBssids = process->readLine();
            seenBssids.remove(seenBssids.length() - 1, 1); //! Remove '\n'

            callback(ssid, seenBssids);
        } else {
            NC_WARN << process->exitCode() << process->readAll() << connName;
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

    auto onFinished = [&](QProcess* pr) {
        if (pr->exitStatus() == QProcess::NormalExit && !pr->exitCode()) {
            QByteArray line = pr->readLine();
            while (!line.isEmpty()) {
                if (line.startsWith("yes:")) {
                    QString bssid = line.sliced(QString("yes:").length());
                    bssid.chop(1);
                    wifiName = bssid;
                    return;
                }
                line = pr->readLine();
            }
        } else {
            qDebug() << "nmcli: pr error: " << pr->errorString();
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

    auto onFinished = [&](QProcess* process) {
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
        auto onFinished = [&] (QProcess* process) {
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
            "modify",
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

#include "Nmcli.h"
#include <QEventLoop>
#include <QTimer>
#include <QUuid>
#include <QDebug>

int Cli::waitLoop(QProcess* process, uint timeout) const
{
    if (timeout <= 0) {
        timeout = NC_WAIT_MSEC;
    }

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

void Cli::preExec(QProcess* process, const QString& command, const QStringList& args, const QString& logline)
{
    qInfo() << "STARTING: " + logline;
    mProcesses.append(process);
    process->setReadChannel(QProcess::StandardOutput);
    connect(process, &QProcess::started, this, [this, logline] () {
        qInfo() << "STARTED: " + logline;
    });
    process->start(command, args);
}

void Cli::postExec(QProcess* process, ExitedCallback callback, const QString& logline)
{
    mProcesses.removeOne(process);
    qInfo() << "FINISHED: " + logline;
    if (process->exitCode() != 0 || process->exitStatus() != QProcess::NormalExit) {
        qWarning() << "ERROR: " + logline + " --> " << process->exitCode() << " / " << process->errorString();
    }
    if (callback) {
        callback(process);
    }
    emit finished(process->exitCode(), process->exitStatus());
}

void Cli::execSync(const QString& command, const QStringList& args, ExitedCallback callback, uint timeout)
{   
    QString logline = QUuid::createUuid().toString() + "# " + command + " " + args.join(' ');
    QProcess process;
    preExec(&process, command, args, logline);
    waitLoop(&process, timeout);
    postExec(&process, callback, logline);
}

void Cli::execAsync(const QString& command, const QStringList& args, ExitedCallback callback, InitCallback init)
{
    QString logline = QUuid::createUuid().toString() + "# " + command + " " + args.join(' ');
    QProcess* process = new QProcess;
    if (init) {
        init(process);
    }
    preExec(process, command, args, logline);
    connect(process, &QProcess::finished, this, [this, process, callback, logline](int exitCode, QProcess::ExitStatus exitStatus) {
        postExec(process, callback, logline);
        delete process;
    }, Qt::SingleShotConnection);
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

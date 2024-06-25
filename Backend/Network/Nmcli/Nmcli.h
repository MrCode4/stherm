#pragma once

#include <QtDebug>
#include <QObject>
#include <QProcess>
#include <QList>

#define NC_COMMAND          "nmcli"
#define NC_ARG_DEVICE       "device"
#define NC_ARG_RADIO        "radio"
#define NC_ARG_NETWORKING   "networking"
#define NC_ARG_GENERAL      "general"
#define NC_ARG_CONNECTION   "connection"

#define NC_ARG_CON_ADD      "add"
#define NC_ARG_WIFI         "wifi"
#define NC_ARG_LIST         "list"
#define NC_ARG_CONNECT      "connect"
#define NC_ARG_DISCONNECT   "disconnect"
#define NC_ARG_DOWN         "down"
#define NC_ARG_UP           "up"
#define NC_ARG_PASSWORD     "password"
#define NC_ARG_SHOW         "show"
#define NC_ARG_DELETE       "delete" //! Used for forgeting a network with NC_ARG_CONNECTION
#define NC_ARG_GET_VALUES   "--get-values"
#define NC_ARG_RESCAN       "--rescan"
#define NC_ARG_FIELDS       "--fields"
#define NC_WAIT_MSEC        500

#define NC_REFERESH_ARGS    QStringList({ "--fields", "IN-USE,BSSID,SSID,SIGNAL,SECURITY" })
#define NC_PRINT_MODE_ARGS  QStringList({ "--mode", "multiline", "--terse" })

//! Some macros for messages recieved from nmcli
#define NC_MSG_DEVICE_OFF               QString(": unavailable")
#define NC_MSG_USING_CONNECTION         QString(": using connection ")
#define NC_MSG_CONNECTION_FAILED        QString(": connection failed")
#define NC_MSG_CONNECTED                QString(" is now the primary connection")
#define NC_MSG_DISCONNECTED             QString(": disconnected")
#define NC_MSG_DISCONNECT_STATE         QString("Networkmanager is now in the 'disconnected' state")
#define NC_MSG_WIFI_PROFILE_REMOVED     QString(": connection profile removed")

//! Printing macros
#define NC_DEBUG            qDebug() << "NMCLI: " << Q_FUNC_INFO << __LINE__
#define NC_WARN             qWarning() << "NMCLI: " << Q_FUNC_INFO << __LINE__
#define NC_CRITICAL         qCritical() << "NMCLI: " << Q_FUNC_INFO << __LINE__

class Cli : public QObject {
    Q_OBJECT

public:
    using ExitedCallback = std::function<void (QProcess* process)>;
    explicit Cli(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

public:
    void execSync(const QString& command, const QStringList& args, ExitedCallback callback, uint timeout);
    void execAsync(const QString& command, const QStringList& args, ExitedCallback callback);
    void kill();

private:
    int waitLoop(QProcess* process, uint timeout) const;
    void preExec(QProcess* process, const QString& command, const QStringList& args, const QString& cmdline);
    void postExec(QProcess* process, ExitedCallback callback, const QString& logline);

private:
    QList<QProcess*> mProcesses;
};

class NmCli : public Cli
{
    Q_OBJECT

public:
    explicit NmCli(QObject* parent = nullptr) : Cli(parent) {}

    void turnWifiDeviceOn(ExitedCallback callback);
    void turnWifiDeviceOff(ExitedCallback callback);
    void refreshWifi(bool rescan, ExitedCallback callback);
    void scanConnectionProfiles(ExitedCallback callback);
    void addConnection(
        const QString& deviceMac,
        const QString& name,
        const QString& ssid,
        const QString& ip4,
        const QString& gw4,
        const QString& dns,
        const QString& security,
        const QString& password,
        ExitedCallback callback);

    void getProfileInfoByName(const QString& connName, std::function<void (const QString&, const QString&)> callback);
    QString getConnectedWifiBssid();
    bool hasWifiProfile(const QString& ssid);

    void connectToUnsavedWifi(const QString& bssid, const QString& password, ExitedCallback callback);
    void connectToSavedWifi(const QString& ssid, const QString& password, ExitedCallback callback);
    void disconnectFromWifi(const QString& ssid, ExitedCallback callback);
    void forgetWifi(const QString& ssid, ExitedCallback callback);
};

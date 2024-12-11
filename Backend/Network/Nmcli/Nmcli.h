#pragma once
#include "Core/ProcessExecutor.h"

#define NC_COMMAND          "nmcli"
#define NC_ARG_DEVICE       "device"
#define NC_ARG_RADIO        "radio"
#define NC_ARG_NETWORKING   "networking"
#define NC_ARG_GENERAL      "general"
#define NC_ARG_CONNECTION   "connection"

#define NC_ARG_ADD          "add"
#define NC_ARG_MODIFY       "modify"
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
#define NC_DEBUG_IF(check)  if (check) qDebug() << "NMCLI: " << Q_FUNC_INFO << __LINE__
#define NC_INFO             qInfo() << "NMCLI: " << Q_FUNC_INFO << __LINE__
#define NC_WARN             qWarning() << "NMCLI: " << Q_FUNC_INFO << __LINE__
#define NC_CRITICAL         qCritical() << "NMCLI: " << Q_FUNC_INFO << __LINE__


class NmCli : public ProcessExecutor
{
    Q_OBJECT

public:
    explicit NmCli(QObject* parent = nullptr) : ProcessExecutor(parent) {}

    void turnWifiDeviceOn(ExitedCallback callback);
    void turnWifiDeviceOff(ExitedCallback callback);
    void refreshWifi(bool rescan, ExitedCallback callback);
    void scanConnectionProfiles(ExitedCallback callback);
    void getDevicePowerState(ExitedCallback callback);
    void getWifiDeviceName(ExitedCallback callback);
    void startMonitoring(InitCallback callback);
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

    void getProfileInfoByNameAsync(const QString& connName, std::function<void (const QString&, const QString&)> callback);
    void getProfileInfoByName(const QString& connName, std::function<void (const QString&, const QString&)> callback);
    QString getConnectedWifiBssid();
    bool hasWifiProfile(const QString& ssid);

    void connectToUnsavedWifi(const QString& bssid, const QString& password, ExitedCallback callback);
    void connectToSavedWifi(const QString& ssid, const QString& security, const QString& password, ExitedCallback callback);
    void disconnectFromWifi(const QString& ssid, ExitedCallback callback);
    void forgetWifi(const QString& ssid, ExitedCallback callback);

private:
    /*!
     * \brief connectToSavedWifiImpl This method is used inside \ref connectToSavedWifi to avoid
     * complicating that method
     * \param ssid
     * \param security
     * \param password
     * \param callback
     */
    void connectToSavedWifiImpl(const QString& ssid, const QString& security, const QString& password, ExitedCallback callback);

    /*!
     * \brief securityToNmcliKeyMgmt convert the security string which can be WAP2, WAP3, WPA2 WPA3
     * to a valid string for 802-11-wireless-security.key-mgmt.
     * \see https://networkmanager.dev/docs/api/latest/settings-802-11-wireless-security.html
     * \param security
     * \return
     */
    QString securityToNmcliKeyMgmt(const QString& security);
};

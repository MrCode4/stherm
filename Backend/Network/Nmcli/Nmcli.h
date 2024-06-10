#pragma once

#include <QtDebug>

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
#define NC_MSG_WIFI_PROFILE_REMOVED     QString(": connection profile removed")

//! Printing macros
#define NC_DEBUG            qDebug() << "NMCLI: " << Q_FUNC_INFO << __LINE__
#define NC_WARN             qWarning() << "NMCLI: " << Q_FUNC_INFO << __LINE__
#define NC_CRITICAL         qCritical() << "NMCLI: " << Q_FUNC_INFO << __LINE__

#pragma once

#include <QObject>
#include "Nmcli.h"

/*!
 * \brief The NmcliObserver class monitors changes to the system network manager using \a\b nmcli
 * tool
 */
class NmcliObserver : public QObject
{
    Q_OBJECT

public:
    explicit NmcliObserver(QObject *parent = nullptr);
    ~NmcliObserver();

    /*!
     * \brief isWifiOn Getter for device power state
     * \return
     */
    bool    isWifiOn() const;

    /*!
     * \brief wifiDevice
     * \return
     */
    QString wifiDevice() const { return mWifiDevice; }

private:
    /*!
     * \brief setDevicePowerState
     * \param isOn
     */
    void    setDevicePowerState(bool isOn);

    /*!
     * \brief getDevicePowerState
     */
    void    getDevicePowerState();

private slots:
    /*!
     * \brief onMonitorProcessReadReady
     */
    void    onMonitorProcessReadReady();

signals:
    /*!
     * \brief wifiIsConnecting This signal is emitted when a wifi is started to be connected to, i.e
     * preparing and configuring states
     * \param ssid The \b ssid of the wifi
     */
    void    wifiIsConnecting(QString ssid);

    /*!
     * \brief connectedToWifi This signal is emitted when a connection to a wifi is successfully made
     * \param bssid
     */
    void    wifiConnected(QString bssid);

    /*!
     * \brief disconnectedFromWifi This signal is emitted when currently connected wifi is
     * successfully disconnected from
     */
    void    wifiDisconnected();

    /*!
     * \brief wifiNeedAuthentication This sigal is emitted when a network needs authentication (for
     * example when entered password was wrong)
     * \param ssid The \b ssid of the wifi
     */
    void    wifiNeedAuthentication(QString ssid);

    /*!
     * \brief wifiForgotten
     * \param ssid
     */
    void    wifiForgotten(QString ssid);

    /*!
     * \brief wifiDevicePowerChanged This signal is emitted when on/off state of wifi device is
     * changed
     * \param on
     */
    void    wifiDevicePowerChanged();

private:
    /*!
     * \brief mMonitorProcess This process is used to monitor nmcli for changes in network
     */
    NmCli*           mCliCommon;
    NmCli*           mCliMonitor;

    /*!
     * \brief mDeviceIsOn
     */
    bool                mDeviceIsOn;

    /*!
     * \brief mWifiDevice This will hold the name of wifi device. Possible values are wlp2s0,
     * wlp1s0, etc.
     */
    QString             mWifiDevice;

    /*!
     * \brief mConnectingWifiSsid Holds the SSID of a wifi that is the NetworkManager is currently
     * tying to connect to.
     */
    QString             mConnectingWifiSsid;
};

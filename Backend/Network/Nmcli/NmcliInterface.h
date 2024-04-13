#pragma once

#include <QEventLoop>
#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QVariant>

#include "NmcliObserver.h"
#include "WifiInfo.h"

//! Aliasing wifis list
using WifisList = QList<WifiInfo*>;

/*!
 * \brief The NmcliInterface class is an interface to hide complexity of interacting with \a \b
 * nmcli.
 * \note Following data are retrieved and stored for all wifi networks:
 *  - IN-USE:       boolean
 *  - BSSID:        string
 *  - SSID:         string
 *  - MODE:         string
 *  - CHAN:         int
 *  - RATE:         int (Mbit/s)
 *  - SIGNAL:       int (0 to 100)
 *  - SECURITY:     string
 */
class NmcliInterface : public QObject
{
    Q_OBJECT

    //! Enums
public:
    //! See nmcli man page
    enum Error {
        Success=0,              //! indicates the operation succeeded.
        UnknownError=1,         //! Unknown or unspecified error.
        Invalid=2,              //! Invalid user input, wrong nmcli invocation.
        Timeout=3,              //! Timeout expired (see --wait option).
        ActivationFailed=4,     //! Connection activation failed.
        DeactivationFailed=5,   //! Connection deactivation failed.
        DisconnectFailed=6,     //! Disconnecting device failed.
        ConnectDeleteFailed=7,  //! Connection deletion failed.
        NotRunning=8,           //! NetworkManager is not running.
        NotExist=10,            //! Connection, device, or access point does not exist.
    };

public:
    explicit NmcliInterface(QObject* parent = nullptr);
    ~NmcliInterface();

    /*!
     * \brief isRunning Determines if a process is already running
     * \return
     */
    bool    isRunning() const;

    /*!
     * \brief getWifis Returns the list of wifis to the caller
     * \return
     */
    WifisList& getWifis();

    /*!
     * \brief refreshWifis This method can be used to refresh wifis.
     * \param rescan If this is true \a\b NetworkManager is forced to rescan and if false it will
     * be automatically decided by \a\b NetworkManager
     */
    void    refreshWifis(bool rescan=false);

    /*!
     * \brief hasWifiProfile Check if a profile for the given wifi is there (i.e wifi password is saved)
     * \param wifi The SSID (and not BSSID) of wifi
     * \param bssid The BSSID to make sure if retrived profile is for this wifi
     * \return
     */
    bool    hasWifiProfile(const WifiInfo* wifi);

    /*!
     * \brief connectToWifi Connects to a wifi network with the given \a bssid (or ssid)
     * \param wifiInfo
     */
    void    connectToWifi(WifiInfo* wifi, const QString& password = "");

    /*!
     * \brief disconnectWifi Disconnects from currently connected wifi
     * \param wifi The SSID (and not BSSID) of wifi
     */
    void    disconnectFromWifi(WifiInfo* wifi);

    /*!
     * \brief connectedWifi Returns currently connected wifi
     * \return
     */
    WifiInfo* connectedWifi();

    /*!
     * \brief forgetWifi Forgets a wifi
     * \param wifi The SSID (and not BSSID) of wifi
     */
    void    forgetWifi(WifiInfo* wifi);

    /*!
     * \brief turnWifiDeviceOn Turn on wifi device
     */
    void    turnWifiDeviceOn();

    /*!
     * \brief turnWifiDeviceOff Turn on wifi device
     */
    void    turnWifiDeviceOff();

    /*!
     * \brief addConnection Add a custom connection
     * \param name
     * \param ssid
     * \param ip4
     * \param gw4
     * \param dns
     * \param security
     * \param password
     */
    void    addConnection(const QString& name,
                       const QString& ssid,
                       const QString& ip4,
                       const QString& gw4,
                       const QString& dns,
                       const QString& security,
                       const QString& password);

    /*!
     * \brief getConnectedWifiBssid
     * \return
     */
    QString getConnectedWifiBssid() const;

private:
   /*!
     * \brief waitLoop
     * \param process
     * \param timeout
     * \return
     */
    int     waitLoop(QProcess *process, int timeout = 1000) const;

    /*!
     * \brief connectToWifi This is an overloaded method and connects to the given wifi without any
     * password, if the given \a ssid is not saved into \a\b NetworkManager this returns
     * immediately otherwise connection process starts
     * \param wifi
     * \param password If password is not empty, first the wifi profile is modified then connection
     * is made
     * \return True if successfull
     * \note This method should be called on a saved wifi. It doesn't check if wifi is saved
     */
    bool    connectSavedWifi(WifiInfo* wifi, const QString& password);

    /*!
     * \brief setupObserver
     */
    void    setupObserver();

    /*!
     * \brief setConnectedWifi
     * \param wifi
     */
    void    setConnectedWifi(WifiInfo* wifi);

    /*!
     * \brief onWifiConnected
     * \param ssid
     */
    void    onWifiConnected(const QString& ssid);

    /*!
     * \brief onWifiDisconnected
     */
    void    onWifiDisconnected();

private slots:
    /*!
     * \brief onGetWifiDeviceNameFinished This slot is used to get the name of wifi device and store
     * it in \a\b mWifiDevice
     * \param exitCode
     * \param exitStatus
     */
    void    onGetWifiDeviceNameFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /*!
     * \brief onWifiListRefreshFinished This slot is connected to \a\b QProcess::finished() as
     * single-shot in \ref refreshWifis(bool) to get wifi lists and emit \ref
     * wifiListRefereshed(WifiListMap) signal
     * \param exitCode
     * \param exitStatus
     */
    void    onWifiListRefreshFinished(int exitCode, QProcess::ExitStatus exitStatus);

signals:
    /*!
     * \brief isRunningChanged This signal is emitted when running state of internal \a\b QProcess
     * is changed
     */
    void    isRunningChanged();

    /*!
     * \brief wifisChanged
     */
    void    wifisChanged();

    /*!
     * \brief connectedWifiChanged
     */
    void    connectedWifiChanged();

    /*!
     * \brief wifiNeedAuthentication
     */
    void    wifiNeedAuthentication();

    /*!
     * \brief errorOccured This signal is emitted when an error is occured
     * \param error
     */
    void    errorOccured(NmcliInterface::Error error);

    /*!
     * \brief deviceIsOnChanged
     */
    void    deviceIsOnChanged();

private:
    /*!
     * \brief mObserver
     */
    NmcliObserver*      mNmcliObserver;

    /*!
     * \brief mProcess The \a\b QProcess that is used to do everything;
     */
    QProcess*           mProcess;

    /*!
     * \brief mWifiDevice This will hold the name of wifi device. Possible values are wlp2s0,
     * wlp1s0, etc.
     */
    QString             mWifiDevice;

    /*!
     * \brief mWifis Stores all the retrieved wifis
     */
    WifisList           mWifis;

    /*!
     * \brief mConnectedWifi Currently connected wifi
     */
    WifiInfo*           mConnectedWifi;

    /*!
     * \brief mDeviceIsOn Holds whether wifi device is on or off
     */
    bool                mDeviceIsOn;
};

#pragma once

#include <QEventLoop>
#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QVariant>
#include <QFileSystemWatcher>

#include "NmcliObserver.h"
#include "WifiInfo.h"

struct ConnectionProfile {
    ConnectionProfile(const QString& s, const QString& bs) : ssid(s), seenBssids(bs) {}

    QString ssid;
    QString seenBssids;
};

class NmCli;

//! Profiles list
using ProfilesList = QList<ConnectionProfile>;

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
        IWFetchError=11
    };

public:
    explicit NmcliInterface(QObject* parent = nullptr);
    ~NmcliInterface();

    /*!
     * \brief busyRefreshing returns true if \ref NmcliInterface is busy refreshing lists
     * \return
     */
    bool    busyRefreshing() const;
    void    setBusyRefreshing(bool busy);

    /*!
     * \brief busy Returns true if \ref NmcliInterface is busy doing something other than refreshing
     * wifis list
     * \return
     */
    bool    busy() const;

    void    setBusy(bool busy);

    /*!
     * \brief isDeviceOn
     * \return
     */
    bool    isDeviceOn() const { return mNmcliObserver->isWifiOn(); }

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

    /*!
     * \brief Gets the list of ciphers supported by the device.
     * \return A QStringList containing the supported ciphers.
     */
    QStringList getDeviceSupportedCiphersList() const;

    bool autoConnectSavedWifiAsync(WifiInfo *wifi);

    QString iwOut() const;

private:
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

    /*!
     * \brief parseBssidToCorrectSsidMap This methods uses iw result to find the correct name of
     * wifi ssids that have some characters unknown to nmcli
     */
    void    parseBssidToCorrectSsidMap(QProcess* process);

    /*!
     * \brief doRefreshWifi Simply performs the nmcli refresh command
     */
    void    doRefreshWifi();

    /*!
     * \brief scanConProfiles Use nmcli to get the list of connections
     */
    void    scanConProfiles();

    /*!
     * \brief initializeConProfilesWatcher
     */
    void    initializeConProfilesWatcher();

    /*!
    * \brief Decodes hexadecimal escape sequences in a string into their corresponding characters.
    *
    * This function scans the provided string for hexadecimal escape sequences (e.g., \x41)
    * and converts them into their corresponding characters (e.g., 'A').
    * It is useful for decoding strings that contain encoded hexadecimal values,
    * commonly found in network SSIDs or other encoded text formats.
    *
    * \param ssid The input QString containing the hexadecimal escape sequences.
    * \return A QString where all hexadecimal escape sequences have been replaced 
    *         by their corresponding characters.
    */

    QString decodeHexToChars(const QString &ssid);
    /*!
     * \brief Reads the supported ciphers of the device using the `iw list` command.
     * 
     * \note This operation is asynchronous and will trigger the `ciphersAreReady()` signal
     * once the ciphers list is obtained.
     */
    void readDeviceSupportedCiphersFromIW();

private slots:
    /*!
     * \brief onWifiListRefreshFinished This slot is connected to \a\b QProcess::finished() as
     * single-shot in \ref refreshWifis(bool) to get wifi lists and emit \ref
     * wifiListRefereshed(WifiListMap) signal
     * \param QProcess
     */
    void    onWifiListRefreshFinished(QProcess* process);

    /*!
     * \brief updateNextConnectionProfile
     */
    void    updateNextConnectionProfile(QSharedPointer<QTextStream> stream, ProfilesList* profiles);

    /*!
     * \brief updateConProfilesList Updates connection profiles list from result of th process
     * started in \ref scanConProfiles()
     */
    void    updateConProfilesList(QProcess* process);

signals:
    /*!
     * \brief busyRefreshing
     */
    void    busyRefreshingChanged();

    /*!
     * \brief busy
     */
    void    busyChanged();

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
    void    wifiNeedAuthentication(WifiInfo* wifi);

    /*!
     * \brief errorOccured This signal is emitted when an error is occured
     * \param error
     */
    void    errorOccured(NmcliInterface::Error error);

    /*!
     * \brief deviceIsOnChanged
     */
    void    deviceIsOnChanged();
    /*!
     * \brief ciphersAreReady
     */
    void ciphersAreReady();

    //! Auto connection finished for wifi.
    void autoConnectSavedInRangeWifiFinished(WifiInfo *wifi);

    void wifiForgotten(WifiInfo* wifi);

private:
    /*!
     * \brief mObserver
     */
    NmcliObserver*          mNmcliObserver;

    NmCli*  mCliCommon;
    NmCli*  mCliProfiles;

    /*!
     * \brief mCliRefresh The \a\b QProcess that is used to do refreshing
     */
    NmCli*  mCliRefresh;

    /*!
     * \brief mWifiProcess is used to do wifi related operations like conneting, disconnecting, etc
     */
    NmCli*  mCliWifi;

    /*!
     * \brief mCliCipers is used to execute the `iw list` command asynchronously.
     */
    NmCli *mCliCipers;

    /*!
     * \brief mIwOut is used to store response of the `iw` command.
     */
    QString mIwOut;

    /*!
     * \brief mWifis Stores all the retrieved wifis
     */
    WifisList               mWifis;

    /*!
     * \brief mSavedWifis This is the list of all the wifi profile saved by NetworkManager
     */
    ProfilesList            mConProfiles;

    /*!
     * \brief mConnectedWifi Currently connected wifi
     */
    WifiInfo*               mConnectedWifi;

    /*!
     * \brief mBssToCorrectSsidMap Since nmcli doesn't return some wifi ssid names correctly (those
     * which has some special chars), it's needed to fix those representations with the help of iw
     * and store them somewhere so it can be used when wifi list refresh finished to correct the
     * ssid of the wifis in nmcli
     */
    QMap<QString, QString>  mBssToCorrectSsidMap;

    /*!
    * \brief mBssToCorrectSecurityMap A map that stores the correct security information for Wi-Fi networks.
    *
    * Since `nmcli` does not always return the correct security type for some Wi-Fi networks (e.g., WPA3 or WPA2),
    * this map is used to associate BSS (Basic Service Set) MAC addresses with their correct security type.
    *
    * The key is a QString representing the BSS MAC address, and the value is a QString representing the correct security type.
    */
    QMap<QString, QString> mBssToCorrectSecurityMap;

    /*!
     * \brief Holds the list of supported ciphers for the device.
     */
    QStringList mDeviceSupportedCiphersList;

    /*!
     * \brief mBusyRefreshing Indicates if it's busy refreshing wifi lists
     */
    bool                    mBusyRefreshing = false;

    /*!
     * \brief mBusy
     */
    bool                    mBusy = false;

    /*!
     * \brief mRescanInRefresh Whether rescan should be forced or not
     */
    bool                    mRescanInRefresh;

    /*!
     * \brief mBusyUpdatingConProfiles If connection profiles are already getting updated.
     */
    bool                    mBusyUpdatingConProfiles = false;

    /*!
     * \brief mConProfilesWatcher This watcher is used to get notified about changes in the wifi
     * connection profiles (only add and remove)
     */
    QFileSystemWatcher*     mConProfilesWatcher;
};

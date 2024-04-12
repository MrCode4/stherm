#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QNetworkInterface>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QTimer>

class NmcliInterface;
class NmcliObserver;

/*!
 * \brief The WifiInfo class holds information of a wifi network
 */
class WifiInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool     connected       MEMBER mConnected       NOTIFY connectedChanged)
    Q_PROPERTY(bool     isConnecting    MEMBER mIsConnecting    NOTIFY isConnectingChanged)
    Q_PROPERTY(int      strength        MEMBER mStrength        NOTIFY strengthChanged)
    Q_PROPERTY(QString  ssid            MEMBER mSsid            NOTIFY ssidChanged)
    Q_PROPERTY(QString  bssid           MEMBER mBssid           NOTIFY bssidChanged)
    Q_PROPERTY(QString  security        MEMBER mSecurity        NOTIFY securityChanged)
    QML_ELEMENT

public:
    explicit WifiInfo(QObject* parent=nullptr) : QObject(parent) {}
    WifiInfo(bool connected, const QString& ssid, const QString& bssid, int strength,
             const QString& security, QObject* parent=nullptr)
        : QObject(parent)
        , mConnected (connected)
        , mStrength(strength)
        , mSsid(ssid)
        , mBssid(bssid)
        , mSecurity(security)
        , mIsConnecting(false)
        {
        };

    bool        mConnected;
    bool        mIsConnecting;
    int         mStrength;
    QString     mSsid;
    QString     mBssid;
    QString     mSecurity;

signals:
    void        connectedChanged();
    void        strengthChanged();
    void        ssidChanged();
    void        bssidChanged();
    void        securityChanged();
    void        isConnectingChanged();
};

/*!
 * \brief The NetworkInterface class provides an interface to fetch all avialable wifi connections
 * on this device and connecting to one of them.
 * \note \ref NetworkInterface is designed to work with \a \b nmcli on linux for more information
 * read it man page.
 */
class NetworkInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<WifiInfo> wifis READ wifis           NOTIFY wifisChanged)
    Q_PROPERTY(bool isRunning                   READ isRunning       NOTIFY isRunningChanged)
    Q_PROPERTY(WifiInfo* connectedWifi          READ connectedWifi   NOTIFY connectedWifiChanged)
    Q_PROPERTY(bool deviceIsOn                  READ deviceIsOn      NOTIFY deviceIsOnChanged)
    Q_PROPERTY(bool hasInternet                 READ hasInternet     NOTIFY hasInternetChanged)
    Q_PROPERTY(QString ipv4Address              READ ipv4Address     CONSTANT)

    QML_ELEMENT
    QML_SINGLETON

    using WifiInfoList = QQmlListProperty<WifiInfo>;

public:
    explicit NetworkInterface(QObject *parent = nullptr);

    /* Public methods
     * ****************************************************************************************/
    WifiInfoList        wifis();

    bool                isRunning();

    WifiInfo*           connectedWifi() const;

    bool                deviceIsOn() const { return mDeviceIsOn; }

    bool                hasInternet() const { return mHasInternet; }

    QString             ipv4Address() const;

    Q_INVOKABLE void    refereshWifis(bool forced = false);
    Q_INVOKABLE void    connectWifi(WifiInfo* wifiInfo, const QString& password);
    Q_INVOKABLE void    connectSavedWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE void    disconnectWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE void    forgetWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE bool    isWifiSaved(WifiInfo* wifiInfo);
    Q_INVOKABLE void    turnOn();
    Q_INVOKABLE void    turnOff();
    Q_INVOKABLE void    sendLog(const QString& serialNo);
    Q_INVOKABLE void    addConnection(const QString& name,
                                   const QString& ssid,
                                   const QString& ip4,
                                   const QString& gw4,
                                   const QString& dns,
                                   const QString& security,
                                   const QString& password);


    /* Private methods and slots
     * ****************************************************************************************/
private:
    static WifiInfo*    networkAt(WifiInfoList* list, qsizetype index);
    static qsizetype    networkCount(WifiInfoList* list);

    /*!
     * \brief setHasInternet
     * \param hasInternet
     */
    inline void         setHasInternet(bool hasInternet);

    /*!
     * \brief setConnectedWifiInfo
     * \param wifiInfo
     */
    inline void         setConnectedWifiInfo(WifiInfo* wifiInfo);

private slots:
    void                onErrorOccured(int error); //! error is: NmcliInterface::Error
    void                onWifiListRefreshed(const QList<QMap<QString, QVariant>>& wifis);
    void                onWifiConnected(const QString& ssid);
    void                onWifiDisconnected();
    void                checkHasInternet();

    /* Signals
     * ****************************************************************************************/
signals:
    void                wifisChanged();
    void                isRunningChanged();
    void                connectedWifiChanged();
    void                deviceIsOnChanged();
    void                hasInternetChanged();
    //!
    //! \brief errorOccured This is a private signal and is emitted when an error occurs during an
    //! opration. The \a ssid param holds name of the wifi network that this error is related to and
    //! is empty if error is not related to a specific wifi network
    //! \param error
    //! \param ssid
    //!
    void                errorOccured(QString error, QString ssid);

    void                incorrectWifiPassword(WifiInfo* wifi);

    /* Private attributes
     * ****************************************************************************************/
private:
    /*!
     * \brief mNmcliInterface An instance of NmcliInterface
     */
    NmcliInterface*         mNmcliInterface;

    /*!
     * \brief mNmcliObserver
     */
    NmcliObserver*          mNmcliObserver;

    /*!
     * \brief mDeviceIsOn Holds whether wifi device is on or off
     */
    bool                    mDeviceIsOn;

    /*!
     * \brief mHasInternet Holds whether there is a wifi connection AND it has full internet access
     */
    bool                    mHasInternet;

    /*!
     * \brief mCheckInternetAccessTmr A timer to check internet access
     */
    QTimer                  mCheckInternetAccessTmr;

    QTimer                  mSetNoInternetTimer;

    /*!
     * \brief cCheckInternetAccessInterval Interval of checking internet access (default: 30 secs)
     */
    const int               cCheckInternetAccessInterval = 30000;

    /*!
     * \brief mCheckInternetAccessUrl The url that is used to check internet access. This is read
     * from env (NMCLI_INTERNET_ACCESS_URL) and 'google.com' is used if it doesn't exist.
     */
    const QUrl              cCheckInternetAccessUrl;

    /*!
     * \brief mWifiInfos List of all the wifis
     */
    QList<WifiInfo*>        mWifiInfos;

    /*!
     * \brief mNam QNetworkRequestManager that is used to check internet access
     */
    QNetworkAccessManager   mNam;

    /*!
     *  \brief mNamIsRunning Whether mNam is running a request
     */
    bool                    mNamIsRunning;

    /*!
     * \brief mConnectedWifiInfo Currently connected wifi
     */
    WifiInfo*               mConnectedWifiInfo;

    /*!
     * \brief mRequestedToConnectedWifi The wifi that is requested to connect to
     */
    WifiInfo*               mRequestedToConnectedWifi;
};

inline void NetworkInterface::setHasInternet(bool hasInternet)
{
    if (mHasInternet != hasInternet) {
        mHasInternet = hasInternet;
        emit hasInternetChanged();
    }
}

inline void NetworkInterface::setConnectedWifiInfo(WifiInfo* wifiInfo)
{
    if (mConnectedWifiInfo == wifiInfo) {
        return;
    }

    if (mConnectedWifiInfo) {
        mConnectedWifiInfo->setProperty("connected", false);
    }

    mConnectedWifiInfo = wifiInfo;
    if (mConnectedWifiInfo) {
        mConnectedWifiInfo->setProperty("connected", true);
    }
    emit connectedWifiChanged();
}

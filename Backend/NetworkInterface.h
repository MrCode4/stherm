#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QNetworkInterface>
#include <QProcess>

class NmcliInterface;

/*!
 * \brief The WifiInfo class holds information of a wifi network
 */
class WifiInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool     connected       MEMBER mConnected       NOTIFY connectedChanged)
    Q_PROPERTY(int      strength        MEMBER mStrength        NOTIFY strengthChanged)
    Q_PROPERTY(QString  ssid            MEMBER mSsid            NOTIFY ssidChanged)
    Q_PROPERTY(QString  bssid           MEMBER mBssid           NOTIFY bssidChanged)
    Q_PROPERTY(QString  security        MEMBER mSecurity        NOTIFY securityChanged)
    QML_ELEMENT
    QML_UNCREATABLE("WifiInfo cannot be created from QML.")

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
        {
        };

    bool        mConnected;
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

    Q_PROPERTY(QQmlListProperty<WifiInfo> wifis READ wifis NOTIFY wifisChanged)
    Q_PROPERTY(bool isRunning           READ isRunning       NOTIFY isRunningChanged)
    Q_PROPERTY(WifiInfo*  connectedWifi READ connectedWifi NOTIFY connectedWifiChanged)
    Q_PROPERTY(bool deviceIsOn          READ deviceIsOn NOTIFY deviceIsOnChanged)

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

    Q_INVOKABLE void    refereshWifis(bool forced = false);
    Q_INVOKABLE void    connectWifi(WifiInfo* wifiInfo, const QString& password);
    Q_INVOKABLE void    connectSavedWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE void    disconnectWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE void    forgetWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE bool    isWifiSaved(WifiInfo* wifiInfo);
    Q_INVOKABLE void    turnOn();
    Q_INVOKABLE void    turnOff();


    /* Private methods and slots
     * ****************************************************************************************/
private:
    static WifiInfo*    networkAt(WifiInfoList* list, qsizetype index);
    static qsizetype    networkCount(WifiInfoList* list);

private slots:
    void                onWifiListRefreshed(const QList<QMap<QString, QVariant>>& wifis);
    void                onWifiConnected(const QString& bssid);
    void                onWifiDisconnected();

    /* Signals
     * ****************************************************************************************/
signals:
    void                wifisChanged();
    void                isRunningChanged();
    void                connectedWifiChanged();
    void                deviceIsOnChanged();
    //!
    //! \brief errorOccured This is a private signal and is emitted when an error occurs during an
    //! opration. The \a ssid param holds name of the wifi network that this error is related to and
    //! is empty if error is not related to a specific wifi network
    //! \param error
    //! \param ssid
    //!
    void                errorOccured(QString error, QString ssid);

    /* Private attributes
     * ****************************************************************************************/
private:
    NmcliInterface*     mNmcliInterface;

    bool                mDeviceIsOn;

    QList<WifiInfo*>    mWifiInfos;

    WifiInfo*           mConnectedWifiInfo;
    WifiInfo*           mRequestedToConnectedWifi;
};

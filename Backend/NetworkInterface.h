#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H

#include <QObject>
#include <QQmlEngine>
#include <QNetworkInterface>
#include <QProcess>


/*!
 * \brief The WifiInfo class holds information of a wifi network
 */
class WifiInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool     connected       MEMBER mConnected       NOTIFY connectedChanged)
    Q_PROPERTY(int      strength        MEMBER mStrength        NOTIFY strengthChanged)
    Q_PROPERTY(QString  ssid            MEMBER mSsid            NOTIFY ssidChanged)
    Q_PROPERTY(QString  security        MEMBER mSecurity        NOTIFY securityChanged)
    QML_ELEMENT
    QML_UNCREATABLE("WifiInfo cannot be created from QML.")

public:
    explicit WifiInfo(QObject* parent=nullptr) : QObject(parent) {}
    WifiInfo(bool connected, const QString& name, int strength, const QString& security,
             QObject* parent=nullptr)
        : QObject(parent)
        , mConnected (connected)
        , mStrength(strength)
        , mSsid(name)
        , mSecurity(security)
        {
        };

    bool        mConnected;
    int         mStrength;
    QString     mSsid;
    QString     mSecurity;

signals:
    void        connectedChanged();
    void        strengthChanged();
    void        ssidChanged();
    void        securityChanged();
};

/*!
 * \brief The NetworkInterface class provides an interface to fetch all avialable wifi connections
 * on this device and connecting to one of them.
 */
class NetworkInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<WifiInfo> wifis READ wifis NOTIFY wifisChanged)
    Q_PROPERTY(bool isRunning        READ isRunning       NOTIFY isRunningChanged)

    QML_ELEMENT
    QML_SINGLETON

    using WifiInfoList = QQmlListProperty<WifiInfo>;

public:
    explicit NetworkInterface(QObject *parent = nullptr);

    /* Public methods
     * ****************************************************************************************/
    WifiInfoList        wifis();

    bool                isRunning();

    Q_INVOKABLE void    refereshWifis(bool forced = false);
    Q_INVOKABLE void    connectWifi(WifiInfo* wifiInfo, const QString& password);


    /* Private methods and slots
     * ****************************************************************************************/
private:
    static WifiInfo*    networkAt(WifiInfoList* list, qsizetype index);
    static qsizetype    networkCount(WifiInfoList* list);

private slots:
    void                onWifiProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void                onWifiConnectFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /* Signals
     * ****************************************************************************************/
signals:
    void                wifisChanged();
    void                isRunningChanged();
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
    QList<WifiInfo*>    mWifiInfos;
    QProcess*           mWifiReadProc;

    WifiInfo*           mRequestedWifiToConnect;
};

#endif // NETWORKINTERFACE_H

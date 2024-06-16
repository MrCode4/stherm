#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QNetworkInterface>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QTimer>

#include "WifiInfo.h"
#include "Nmcli/NmcliInterface.h"


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
    Q_PROPERTY(bool busyRefreshing              READ busyRefreshing  NOTIFY busyRefreshingChanged)
    Q_PROPERTY(bool busy                        READ busy            NOTIFY busyChanged)
    Q_PROPERTY(WifiInfo* connectedWifi          READ connectedWifi   NOTIFY connectedWifiChanged)
    Q_PROPERTY(bool deviceIsOn                  READ deviceIsOn      NOTIFY deviceIsOnChanged)
    Q_PROPERTY(bool hasInternet                 READ hasInternet     NOTIFY hasInternetChanged)
    Q_PROPERTY(QString ipv4Address              READ ipv4Address     CONSTANT)

    QML_ELEMENT
    QML_SINGLETON

    using WifisQmlList = QQmlListProperty<WifiInfo>;

public:
    explicit NetworkInterface(QObject *parent = nullptr);

    /* Public methods
     * ****************************************************************************************/
    WifisQmlList        wifis();

    /*!
     * \brief see \ref NmcliInterface::busyRefreshing()
     */
    bool    busyRefreshing() const { return mNmcliInterface && mNmcliInterface->busyRefreshing(); }

    /*!
     * \brief see \ref NmcliInterface::busy()
     */
    bool    busy() const { return mNmcliInterface && mNmcliInterface->busy(); }

    WifiInfo*           connectedWifi() const;

    bool                deviceIsOn() const { return mNmcliInterface->isDeviceOn(); }

    bool                hasInternet() const { return mHasInternet; }

    QString             ipv4Address() const;

    Q_INVOKABLE void    refereshWifis(bool forced = false);
    Q_INVOKABLE void    connectWifi(WifiInfo* wifiInfo, const QString& password = "");
    Q_INVOKABLE void    disconnectWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE void    forgetWifi(WifiInfo* wifiInfo);
    Q_INVOKABLE bool    isWifiSaved(WifiInfo* wifiInfo);
    Q_INVOKABLE void    turnOn();
    Q_INVOKABLE void    turnOff();
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
    static WifiInfo*    networkAt(WifisQmlList* list, qsizetype index);
    static qsizetype    networkCount(WifisQmlList* list);

    /*!
     * \brief setHasInternet
     * \param hasInternet
     */
    inline void         setHasInternet(bool hasInternet);

private slots:
    void                onErrorOccured(int error); //! error is: NmcliInterface::Error
    void                checkHasInternet();

    /* Signals
     * ****************************************************************************************/
signals:
    void                wifisChanged();
    void                busyRefreshingChanged();
    void                busyChanged();
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
     * \brief mWifiInfos List of all the wifis. This is just a shortcut for accessing
     * NmcliInterface::wifis
     */
    QList<WifiInfo*>&       mWifiInfos;

    /*!
     * \brief mNam QNetworkRequestManager that is used to check internet access
     */
    QNetworkAccessManager   mNam;

    /*!
     *  \brief mNamIsRunning Whether mNam is running a request
     */
    bool                    mNamIsRunning;

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

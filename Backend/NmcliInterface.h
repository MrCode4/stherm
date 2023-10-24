#pragma once

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QVariant>


#define NC_COMMAND          "nmcli"
#define NC_ARG_DEVICE       "device"
#define NC_ARG_WIFI         "wifi"
#define NC_ARG_LIST         "list"
#define NC_ARG_CONNECT      "connect"
#define NC_ARG_DISCONNECT   "disconnect"
#define NC_ARG_PASSWORD     "password"
#define NC_ARG_RESCAN       "--rescan"
#define NC_ARG_FIELDS       "--fields"


//! Aliasing wifi info list structure
using WifiListMap = QList<QMap<QString, QVariant>>;

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
        ActiveFailed=4,         //! Connection activation failed.
        DeactiveFailed=5,       //! Connection deactivation failed.
        DisconnectFailed=6,     //! Disconnecting device failed.
        ConnectDeleteFailed=7,  //! Connection deletion failed.
        NotRunning=8,           //! NetworkManager is not running.
        NotExist=10,            //! Connection, device, or access point does not exist.
    };

public:
    explicit NmcliInterface(QObject* parent = nullptr);

    /*!
     * \brief isRunning Determines if a process is already running
     * \return
     */
    bool    isRunning() const;

    /*!
     * \brief refreshWifis This method can be used to refresh wifis.
     * \param rescan If this is true \a\b NetworkManager is forced to rescan and if false it will
     * be automatically decided by \a\b NetworkManager
     */
    void    refreshWifis(bool rescan=false);

    /*!
     * \brief connectToWifi Connects to a wifi network with the given \a bssid (or ssid)
     * \param bssid
     */
    void    connectToWifi(const QString& bssid, const QString& password);

    /*!
     * \brief disconnectWifi Disconnects from currently connected wifi
     */
    void    disconnectWifi();

    /*!
     * \brief forgetWifi Forgets a wifi
     * \param bssid
     */
    void    forgetWifi(const QString& bssid);

    /*!
     * \brief turnWifiDeviceOn Turn on wifi device
     */
    void    turnWifiDeviceOn();

    /*!
     * \brief turnWifiDeviceOff Turn on wifi device
     */
    void    turnWifiDeviceOff();

private slots:
    /*!
     * \brief onWifiListRefreshFinished This slot is connected to \a\b QProcess::finished() as
     * single-shot in \ref refreshWifis(bool) to get wifi lists and emit \ref
     * wifiListRefereshed(WifiListMap) signal
     * \param exitCode
     * \param exitStatus
     */
    void    onWifiListRefreshFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /*!
     * \brief onWifiConnectedFinished This slot is connected to \a\b QProcess::finished() as
     * single-shot in \ref connectToWifi(QString) to propaget that a connection is finished
     * successfully or not
     * \param exitCode
     * \param exitStatus
     */
    void    onWifiConnectedFinished(int exitCode, QProcess::ExitStatus exitStatus);

signals:
    /*!
     * \brief isRunningChanged This signal is emitted when running state of internal \a\b QProcess
     * is changed
     */
    void    isRunningChanged();

    /*!
     * \brief wifiListRefereshed This signal is emitted when wifi list are refereshed carrying out
     * a list of \a\b QMap holding wifi information.
     * \param wifis
     */
    void    wifiListRefereshed(WifiListMap wifis);

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
     * \brief wifiDevicePowerChanged This signal is emitted when on/off state of wifi device is
     * changed
     * \param on
     */
    void    wifiDevicePowerChanged(bool on);

    /*!
     * \brief errorOccured This signal is emitted when an error is occured
     * \param error
     */
    void    errorOccured(NmcliInterface::Error error);

private:
    /*!
     * \brief mProcess The \a\b QProcess that is used to do everything;
     */
    QProcess*           mProcess;

    /*!
     * \brief mRequestedWifiToConnect Holds the SSID/BSSID of a wifi that is requested to connect
     */
    QString             mRequestedWifiToConnect;

    /*!
     * \brief cWifiInfoFields
     */
    const QStringList   cWifiListFieldsArg = { "--fields", "IN-USE,BSSID,SSID,MODE,CHAN,RATE,SIGNAL,SECURITY" };

    /*!
     * \brief cNmcliPrintMode
     */
    const QStringList   cPrintModeArg = { "--mode", "multiline", "--terse" };
};

//! Methods implementations

inline NmcliInterface::NmcliInterface(QObject* parent)
    : QObject { parent }
    , mProcess { new QProcess(this) }
{
    mProcess->setReadChannel(QProcess::StandardOutput);
    connect(mProcess, &QProcess::stateChanged, this, &NmcliInterface::isRunningChanged);
}

inline bool NmcliInterface::isRunning() const
{
    return mProcess && (mProcess->state() == QProcess::Starting
                        || mProcess->state() == QProcess::Running);
}

inline void NmcliInterface::refreshWifis(bool rescan)
{
    if (!mProcess) {
        return;
    }

    if (isRunning()) {
        return;
    }

    connect(mProcess, &QProcess::finished, this, &NmcliInterface::onWifiListRefreshFinished,
            Qt::SingleShotConnection);

    const QStringList args = cWifiListFieldsArg + cPrintModeArg + QStringList({
                                 NC_ARG_DEVICE,
                                 NC_ARG_WIFI,
                                 NC_ARG_LIST,
                                 NC_ARG_RESCAN,
                                 rescan ? "yes" : "auto"
                             });

    mProcess->start(NC_COMMAND, args);
}

inline void NmcliInterface::connectToWifi(const QString& bssid, const QString& password)
{
    if (!mProcess) {
        return;
    }

    if (isRunning()) {
        return;
    }

    mRequestedWifiToConnect = bssid;
    connect(mProcess, &QProcess::finished, this, &NmcliInterface::onWifiConnectedFinished,
            Qt::SingleShotConnection);

    //! Perform connection command
    const QStringList args({
        NC_ARG_DEVICE,
        NC_ARG_WIFI,
        NC_ARG_CONNECT,
        bssid,
        NC_ARG_PASSWORD,
        password,
    });

    mProcess->start(NC_COMMAND, args);
}

inline void NmcliInterface::onWifiListRefreshFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0) {
        WifiListMap wifis;

        //! nmcli results are each on one line.do
        QString line = mProcess->readLine(); //! Holds IN-USE of first wifi info in any
        line.remove(line.length() - 1, 1); //! Remove '\n'

        while (!line.isEmpty()) {
            QMap<QString, QVariant> wifi;

            //! line is like : IN-USE:* (or no *)
            const int inUseLen = 7; //! Length of 'IN-USE:' string
            wifi["inUse"] = QVariant(line.size() > inUseLen ? (line.sliced(inUseLen) == "*") : false );

            //! Read BSSID line: it's in this form: BSSID:<bssid>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int bssidLen = 6; //! Plus one for :
            wifi["bssid"] = line.size() > bssidLen ? line.sliced(bssidLen) : "";

            //! Read SSID line: it's in this form: SSID:<ssid>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int ssidLen = 5;
            wifi["ssid"] = line.size() > ssidLen ? line.sliced(ssidLen) : "";

            //! Read MODE line: it's in this form: MODE:<mode>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int modeLen = 5;
            wifi["mode"] = line.size() > modeLen ? line.sliced(modeLen) : "";

            //! Read CHAN line: it's in this form: CHAN:<chan>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int chanLen = 5;
            wifi["chan"] = line.size() > chanLen ? line.sliced(chanLen) : "";

            //! Read RATE line: it's in this form: RATE:<rate>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int rateLen = 5;
            wifi["rate"] = line.size() > rateLen ? line.sliced(rateLen) : "";

            //! Read SIGNAL line: it's in this form: SIGNAL:<signal>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int signalLen = 6;
            wifi["signal"] = line.size() > signalLen ? line.sliced(signalLen) : "";

            //! Read SECURITY line: it's in this form: SECURITY:<security>
            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
            const int securityLen = 9;
            wifi["security"] = line.size() > securityLen ? line.sliced(securityLen) : "";

            wifis.append(wifi);

            line = mProcess->readLine();
            line.remove(line.length() - 1, 1); //! Remove '\n'
        }

        emit wifiListRefereshed(wifis);
    } else {
        qCritical() << "Error occured : " << mProcess->readAllStandardError();
    }
}

inline void NmcliInterface::onWifiConnectedFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0) {
        //! Connection was successful
        emit wifiConnected(mRequestedWifiToConnect);
        mRequestedWifiToConnect = "";
    } else {
        //! Connection failed
        emit errorOccured(NmcliInterface::Error(exitCode));
    }
}

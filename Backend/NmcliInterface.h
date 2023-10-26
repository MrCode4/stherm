#pragma once

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QVariant>


#define NC_COMMAND          "nmcli"
#define NC_ARG_DEVICE       "device"
#define NC_ARG_RADIO        "radio"
#define NC_ARG_NETWORKING   "networking"
#define NC_ARG_GENERAL      "general"
#define NC_ARG_CONNECTION   "connection"

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
    ~NmcliInterface() {
        if (isRunning()) {
            mProcess->kill();
        }
    }

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
     * \brief hasWifiProfile Check if a profile for the given wifi is there (i.e wifi password is saved)
     * \param ssid The SSID (and not BSSID) of wifi
     * \param bssid The BSSID to make sure if retrived profile is for this ssid
     * \return
     */
    bool    hasWifiProfile(const QString& ssid, const QString& bssid);

    /*!
     * \brief connectToWifi Connects to a wifi network with the given \a bssid (or ssid)
     * \param bssid
     */
    void    connectToWifi(const QString& bssid, const QString& password);

    /*!
     * \brief connectToWifi This is an overloaded method and connects to the given wifi without any
     * password, if the given \a ssid is not saved into \a\b NetworkManager this returns
     * immediately otherwise connection process starts
     * \param ssid The SSID (and not BSSID) of wifi
     * \param bssid The BSSID of wifi
     * \return False if wifi profile with this \a ssid doesn't exit
     * \note This methods uses \ref hasWifiProfile(const QStrig&) to check if \a ssid is saved.
     */
    bool    connectSavedWifi(const QString& ssid, const QString& bssid);

    /*!
     * \brief disconnectWifi Disconnects from currently connected wifi
     * \param ssid The SSID (and not BSSID) of wifi
     */
    void    disconnectFromWifi(const QString& ssid);

    /*!
     * \brief forgetWifi Forgets a wifi
     * \param ssid The SSID (and not BSSID) of wifi
     */
    void    forgetWifi(const QString& ssid);

    /*!
     * \brief turnWifiDeviceOn Turn on wifi device
     */
    void    turnWifiDeviceOn();

    /*!
     * \brief turnWifiDeviceOff Turn on wifi device
     */
    void    turnWifiDeviceOff();

private:
    /*!
     * \brief initialize Gets initial information about wifi device
     */
    void    initialize()
    {
        //! Use another instance of QProcess to avoid possible possible immediate call to \ref
        //! refreshWifis() failing.
        QProcess* process = new QProcess(this);

        //! First check wifi device on/off state
        connect(process, &QProcess::finished, this, [this, process](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0) {
                    emit wifiDevicePowerChanged(process->readLine().contains("enabled"));
                }

                //! Now get device wifi name
                getWifiDeviceName(process);
            }, Qt::SingleShotConnection);

        process->start(NC_COMMAND, {
                                       NC_ARG_GET_VALUES,
                                       "WIFI",
                                       NC_ARG_GENERAL,
                                       "status",
                                   });
    }

    void    getWifiDeviceName(QProcess* process)
    {
        //! Get wifi device name
        connect(process, &QProcess::finished, this,
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
                onGetWifiDeviceNameFinished(exitCode, exitStatus);

                //! Delete process
                process->deleteLater();
            }, Qt::SingleShotConnection);

        process->start(NC_COMMAND, {
                                       NC_ARG_GET_VALUES,
                                       "GENERAL.TYPE,GENERAL.DEVICE",
                                       NC_ARG_DEVICE,
                                       NC_ARG_SHOW,
                                   });
    }

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
     * \brief wifiForgotten
     * \param ssid
     */
    void    wifiForgotten(QString ssid);

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
     * \brief mWifiDevice This will hold the name of wifi device. Possible values are wlp2s0,
     * wlp1s0, etc.
     */
    QString             mWifiDevice;

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

    initialize();
}

inline bool NmcliInterface::isRunning() const
{
    return mProcess && (mProcess->state() == QProcess::Starting
                        || mProcess->state() == QProcess::Running);
}

inline void NmcliInterface::refreshWifis(bool rescan)
{
    if (!mProcess || isRunning()) {
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

inline bool NmcliInterface::hasWifiProfile(const QString& ssid, const QString& bssid)
{
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.start(NC_COMMAND, {
                                  NC_ARG_GET_VALUES,
                                  "802-11-wireless.seen-bssids",
                                  "--escape",
                                  "no",
                                  NC_ARG_CONNECTION,
                                  NC_ARG_SHOW,
                                  ssid
                              });
    process.waitForFinished(100);
    if (process.exitCode() == 0) {
        //! Profile is saved
        return process.readLine() == bssid + "\n";
    }

    return false;
}

inline void NmcliInterface::connectToWifi(const QString& bssid, const QString& password)
{
    if (!mProcess || isRunning()) {
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

inline bool NmcliInterface::connectSavedWifi(const QString& ssid, const QString& bssid)
{
    if (!hasWifiProfile(ssid, bssid)) {
        return false;
    }

    //! Profile exist, connect to it.
    mRequestedWifiToConnect = ssid;
    connect(mProcess, &QProcess::finished, this, &NmcliInterface::onWifiConnectedFinished,
            Qt::SingleShotConnection);

    //! Perform connection command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_UP,
        ssid,
    });

    mProcess->start(NC_COMMAND, args);
    return true;
}

inline void NmcliInterface::disconnectFromWifi(const QString& ssid)
{
    if (!mProcess || isRunning()) {
        return;
    }

    connect(mProcess, &QProcess::finished, this, [&](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) {
                emit wifiDisconnected();
            } else {
                emit errorOccured(NmcliInterface::Error(exitCode));
            }
        }, Qt::SingleShotConnection);

    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DOWN,
        ssid,
    });

    mProcess->start(NC_COMMAND, args);
}

inline void NmcliInterface::forgetWifi(const QString& ssid)
{
    if (!mProcess || isRunning()) {
        return;
    }

    connect(mProcess, &QProcess::finished, this, [&, ssid](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) {
                emit wifiForgotten(ssid);
            } else {
                emit errorOccured(NmcliInterface::Error(exitCode));
            }
        }, Qt::SingleShotConnection);

    //! Perform disconnect command
    const QStringList args({
        NC_ARG_CONNECTION,
        NC_ARG_DELETE,
        ssid,
    });

    mProcess->start(NC_COMMAND, args);
}

inline void NmcliInterface::turnWifiDeviceOn()
{
    if (!mProcess || isRunning()) {
        return;
    }

    connect(mProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            emit wifiDevicePowerChanged(true);
        } else {
            emit errorOccured(NmcliInterface::Error(exitCode));
        }
    }, Qt::SingleShotConnection);

    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "on"
    });

    mProcess->start(NC_COMMAND, args);
}

inline void NmcliInterface::turnWifiDeviceOff()
{
    if (!mProcess || isRunning()) {
        return;
    }

    connect(mProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) {
                emit wifiDevicePowerChanged(false);
            } else {
                emit errorOccured(NmcliInterface::Error(exitCode));
            }
        }, Qt::SingleShotConnection);

    //! Perform connection command
    const QStringList args({
        NC_ARG_RADIO,
        NC_ARG_WIFI,
        "off"
    });

    mProcess->start(NC_COMMAND, args);
}

inline void NmcliInterface::onGetWifiDeviceNameFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0) {
        QByteArray line = mProcess->readLine();
        while (!line.isEmpty() && line != "wifi\n") {
            line = mProcess->readLine();
        }

        mWifiDevice = mProcess->readLine();
        if (!mWifiDevice.isEmpty()) {
            mWifiDevice.remove(mWifiDevice.size() - 1, 1);
        }
    }
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
            const int signalLen = 7;
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
        emit errorOccured(NmcliInterface::Error(exitCode));
    }
}

inline void NmcliInterface::onWifiConnectedFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode == 0 && !mProcess->readLine().startsWith("Error")) {
        //! Connection was successful
        emit wifiConnected(mRequestedWifiToConnect);
        mRequestedWifiToConnect = "";
    } else {
        //! Connection failed
        emit errorOccured(NmcliInterface::Error(exitCode));
    }
}

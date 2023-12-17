#pragma once

#include <QObject>
#include <QtNetwork>

#include "Backend/Device/nuve_types.h"
#include "NetworkWorker.h"

/*! ***********************************************************************************************
 * This class manage system requests.
 * ************************************************************************************************/
namespace NUVE {
class System : public NetworkWorker
{
    Q_OBJECT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/

    System(QObject *parent = nullptr);

    /* Public Functions (Setters and getters)
     * ****************************************************************************************/

    //! Reboot device
    void rebootDevice();

    //! Get technic's url and serial number
    void getQR(QString accessUid) { getSN(accessUid.toStdString()); }

    // TODO review if this, and others below, should be static
    std::string getSN(cpuid_t accessUid);

    //! Get update
    //! todo: process response packet
    //! TEMP: "022"
    void getUpdate(QString softwareVersion = "022");

    //! Send request job to web server
    void requestJob(QString type);

protected slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

signals:
    void snReady();

    //! progress is percentage
    void downloadProgress(int progress);

private:
    //! Start the partilally update
    void partialUpdate(const QJsonObject &jsonObj);

private:
    QString mSerialNumber;
};

} // namespace NUVE

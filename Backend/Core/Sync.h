#pragma once

#include <QObject>
#include <QtNetwork>

#include "nuve_types.h"
#include "NetworkWorker.h"

/*! ***********************************************************************************************
 * This class manage sync requests.
 * ************************************************************************************************/
namespace NUVE {
class Sync : public NetworkWorker
{
    Q_OBJECT

public:
    Sync(QObject *parent = nullptr);

    void changeContractorInfo(QString serialNumber);

    //! Get serial number from server if not fetched or saved
    std::string getSN(cpuid_t accessUid);
    //! returns last fetched from save or server
    std::string getSN();

signals:
    void snReady();

private slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

private:
    /* Attributes
     * ****************************************************************************************/

    bool mIsGetSNReceived;
    QString mSerialNumber;

    void sendGetRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QString &method = "");
};
}

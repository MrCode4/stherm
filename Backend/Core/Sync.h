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

    //! Get serial number and if has client from server if not fetched or saved
    std::pair<std::string, bool> getSN(cpuid_t accessUid);
    //! returns last fetched from save or server
    std::pair<std::string, bool> getSN();

    QVariantMap getContractorInfo();
    void getSettings();
    void getWirings(cpuid_t accessUid);
    void requestJob(QString type);

signals:
    void snReady();
    void wiringReady();
    void contractorInfoReady();
    void settingsLoaded();
    void requestJobDone();

    void alert(QString msg);

private slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

private:
    /* Attributes
     * ****************************************************************************************/

    bool mIsGetSNReceived;
    bool mHasClient;
    QString mSerialNumber;
    QVariantMap mContractorInfo;

    void sendGetRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QString &method = "");
};
}

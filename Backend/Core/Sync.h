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

    void setUID(cpuid_t accessUid);
    //! Get serial number and if has client from server if not fetched or saved
    std::pair<std::string, bool> getSN(cpuid_t accessUid);
    //! returns last fetched from save or server
    std::pair<std::string, bool> getSN();

    QVariantMap getContractorInfo();
    bool getSettings();
    void getMessages();
    void getWirings(cpuid_t accessUid);
    void requestJob(QString type);

    void pushSettingsToServer(const QVariantMap &settings);
    void pushAlertToServer(const QVariantMap &settings);

    void ForgetDevice();

signals:
    void snReady();
    void wiringReady();
    void contractorInfoReady();
    void settingsLoaded();
    void settingsReady(QVariantMap settings);
    void messagesLoaded();
    void requestJobDone();

    void alert(QString msg);

    void pushSuccess();
    void pushFailed();

private slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply) override;

protected:
    void sendGetRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QString &method = "");
    void sendPostRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QByteArray &postData, const QString &method) override;

private:
    /* Attributes
     * ****************************************************************************************/
    bool mHasClient;
    QString mSerialNumber;
    QVariantMap mContractorInfo;

    cpuid_t mSystemUuid;
};
}

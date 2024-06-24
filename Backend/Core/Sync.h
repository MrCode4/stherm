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
    std::pair<std::string, bool> getSN(cpuid_t accessUid, bool notifyUser = true);
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

    bool getAutoModeSetings();

    //! Push auto mode settings to server
    void pushAutoSettingsToServer(const double &auto_temp_low, const double &auto_temp_high);

signals:
    void snReady();
    void wiringReady();
    void contractorInfoReady();
    void settingsLoaded();

    //! Settings data
    void settingsReady(QVariantMap settings);

    //! Parse non-settings data, to update immediately
    void appDataReady(QVariantMap data);

    void autoModeSettingsReady(QVariantMap settings, bool isValid);
    void messagesLoaded();
    void requestJobDone();

    //! Received settings that are invalid and do not require updating the model.
    //! But The request for settings (getSettings) retrieval was successful.
    void invalidSettingsReceived();

    void alert(QString msg);

    void pushSuccess();
    void pushFailed();

    void autoModePush(bool isSuccess);

    void serialNumberChanged();

    void testModeStarted();

protected:
    QNetworkReply* sendGetRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QString &method = "");
    void sendPostRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QByteArray &postData, const QString &method);
    void processNetworkReply(QNetworkReply* reply) override;

private:
    QByteArray preparePacket(QString className, QString method, QJsonArray params);

private:
    /* Attributes
     * ****************************************************************************************/
    bool mHasClient;
    QString mSerialNumber;
    QDateTime mLastPushTime;
    QVariantMap mContractorInfo;

    cpuid_t mSystemUuid;
};
}

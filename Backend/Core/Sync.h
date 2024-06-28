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
    using ResponseCallback = std::function<void (QNetworkReply* reply, const QByteArray& rawData, QJsonObject& data)>;

public:
    Sync(QObject *parent = nullptr);

    void setUID(cpuid_t accessUid);
    //! returns last fetched from save or server
    QString getSerialNumber() const;
    bool hasClient() const;
    //! Get serial number and if has client from server if not fetched or saved
    void fetchSerialNumber(cpuid_t accessUid, bool notifyUser = true);

    QVariantMap getContractorInfo() const;
    void fetchContractorInfo();
    void fetchContractorLogo(const QString& url);
    void fetchSettings();
    void getMessages();
    void getWirings(cpuid_t accessUid);
    void requestJob(QString type);

    void pushSettingsToServer(const QVariantMap &settings);
    void pushAlertToServer(const QVariantMap &settings);

    void ForgetDevice();

    void fetchAutoModeSetings();

    //! Push auto mode settings to server
    void pushAutoSettingsToServer(const double &auto_temp_low, const double &auto_temp_high);

signals:
    void settingsFetched(bool success);
    void serialNumberReady();

    //! Use snFinished signal to exit from sn loop
    void snFinished();

    void wiringReady();
    void contractorInfoReady();

    //! Settings data
    void settingsReady(QVariantMap settings);

    //! Parse non-settings data, to update immediately
    void appDataReady(QVariantMap data);

    void autoModeSettingsReady(const QVariantMap& settings, bool isValid);
    void autoModeSettingsFetched();
    void messagesLoaded();
    void requestJobDone();

    void alert(QString msg);

    void pushSuccess();
    void pushFailed();

    void autoModePush(bool isSuccess);

    void serialNumberChanged();

    void testModeStarted();

protected:
    void processNetworkReply(QNetworkReply* reply) override;

private:
    QByteArray preparePacket(QString className, QString method, QJsonArray params);
    QNetworkRequest prepareApiRequest(const QString& endpoint, bool setAuth = true);
    QNetworkReply* callGetApi(const QString& endpoint, ResponseCallback callback = nullptr, bool setAuth = true);
    QNetworkReply* callPostApi(const QString& endpoint, const QByteArray &postData, ResponseCallback callback = nullptr);

private:
    QHash <QString, ResponseCallback> mCallbacks;
    bool mHasClient;
    QString mSerialNumber;
    QDateTime mLastPushTime;
    QVariantMap mContractorInfo;
    cpuid_t mSystemUuid;
};
}

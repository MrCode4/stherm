#pragma once

#include <QObject>
#include <QtNetwork>

#include "nuve_types.h"
#include "RestApiExecutor.h"

/*! ***********************************************************************************************
 * This class manage sync requests.
 * ************************************************************************************************/
namespace NUVE {
class Sync : public RestApiExecutor
{
    Q_OBJECT    
public:
    Sync(QObject *parent = nullptr);

    void setUID(cpuid_t accessUid);
    //! returns last fetched from save or server
    QString getSerialNumber() const;
    bool hasClient() const;
    QVariantMap getContractorInfo() const;

    void fetchSerialNumber(const QString& uid, bool notifyUser = true);
    void fetchContractorInfo();
    void fetchContractorLogo(const QString& url);
    void fetchSettings();
    void fetchAutoModeSetings();

    void fetchMessages();
    void fetchWirings(const QString& uid);
    void requestJob(QString type);

    void pushSettingsToServer(const QVariantMap &settings);
    void pushAlertToServer(const QVariantMap &settings);

    void forgetDevice();    

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

    void updateFirmwareFromServer(QString version);

private slots:
    //! Check firmware update with getSettings reply
    void checkFirmwareUpdate(QJsonObject settings);

private:
    QByteArray preparePacket(QString className, QString method, QJsonArray params);

protected:    
    void setApiAuth(QNetworkRequest& request) override;
    QJsonObject prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const override;

private:    
    bool mHasClient;
    QString mSerialNumber;
    QDateTime mLastPushTime;
    QVariantMap mContractorInfo;
    cpuid_t mSystemUuid;
};
}

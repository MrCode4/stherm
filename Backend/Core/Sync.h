#pragma once

#include <QObject>
#include <QtNetwork>
#include <QQmlEngine>

#include "nuve_types.h"
#include "RestApiExecutor.h"
#include "Property.h"

/*! ***********************************************************************************************
 * This class manage sync requests.
 * ************************************************************************************************/

namespace NUVE {
class Sync : public RestApiExecutor
{
    Q_OBJECT
    QML_ELEMENT

    //! useful for showing busy indicator when needed
    PROPERTY_PRI(bool, fetchingUserData)
public:
    Sync(QObject *parent = nullptr);

    void setUID(cpuid_t accessUid);
    void setSerialNumber(const QString &serialNumber);
    //! returns last fetched from save or server
    QString getSerialNumber() const;
    bool hasClient() const;
    QVariantMap getContractorInfo() const;

    void fetchSerialNumber(const QString& uid, bool notifyUser = true);
    bool fetchContractorInfo();
    void fetchContractorLogo(const QString& url);
    bool fetchSettings();
    bool fetchAutoModeSetings(bool success);

    bool fetchMessages();
    void fetchWirings(const QString& uid);
    void requestJob(QString type);
    Q_INVOKABLE void fetchUserData();

    void pushSettingsToServer(const QVariantMap &settings);
    void pushAlertToServer(const QVariantMap &settings);

    void forgetDevice();

    //! Push auto mode settings to server
    void pushAutoSettingsToServer(const double &auto_temp_low, const double &auto_temp_high);

    void fetchServiceTitanInformation();

    void warrantyReplacement(const QString& oldSN, const QString& newSN);

    //! Get job information with the job id
    Q_INVOKABLE void getJobIdInformation(const QString &jobID);

signals:
    void settingsFetched(bool success);
    void serialNumberReady();

    //! Use snFinished signal to exit from sn loop
    void snFinished();

    void wiringReady();

    //! If data is successfully retrieved from the server, the contractor information will be updated with the new data.
    //! otherwise the device will use the local informatio
    void contractorInfoReady(bool getDataFromServerSuccessfully = true);
    void userDataFetched(const QString& email, const QString& name);

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

    void serviceTitanInformationReady(bool hasError = true,
                                      bool isActive = false,
                                      QString email = QString(),
                                      QString zipCode = QString());

    //! TODO: send new data to device controller
    //! maybe rename to warrantyReplacementDataReady
    void warrantyReplacementFinished(bool success = false);

    void jobInformationReady(bool success, QVariantMap data);

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
    QDateTime mAutoModeLastPushTime;
    QVariantMap mContractorInfo;
    cpuid_t mSystemUuid;
};
}

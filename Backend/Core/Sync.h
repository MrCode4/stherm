#pragma once

#include <QObject>
#include <QtNetwork>
#include <QQmlEngine>

#include "DevApiExecutor.h"
#include "nuve_types.h"
#include "Property.h"

/*! ***********************************************************************************************
 * This class manage sync requests.
 * ************************************************************************************************/

namespace NUVE {
class Sync : public DevApiExecutor
{
    Q_OBJECT
    QML_ELEMENT

    PROPERTY_PRI(bool, fetchingUserData)
    PROPERTY_PRI(bool, pushingLockState)
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

    Q_INVOKABLE QString baseURL();

    Q_INVOKABLE void pushLockState(const QString& pin, bool lock);

    void pushSettingsToServer(const QVariantMap &settings);
    void pushAlertToServer(const QVariantMap &settings);

    void forgetDevice();

    //! Push auto mode settings to server
    void pushAutoSettingsToServer(const double &auto_temp_low, const double &auto_temp_high);

    void fetchServiceTitanInformation();

    Q_INVOKABLE void warrantyReplacement(const QString& oldSN, const QString& newSN);

    //! Get job information with the job id
    Q_INVOKABLE void getJobIdInformation(const QString &jobID);

    //! Get job information manually with email and zip code
    Q_INVOKABLE void getCustomerInformationManual(const QString& email);
    Q_INVOKABLE void getAddressInformationManual(const QString& zipCode);


    Q_INVOKABLE void installDevice(const QVariantMap &data);

    void getOutdoorTemperature();

    Q_INVOKABLE void clearSchedule(const int &scheduleID);

    Q_INVOKABLE void editSchedule(const int &scheduleID, const QVariantMap &schedule);

    Q_INVOKABLE void addSchedule(const QString &scheduleUid, const QVariantMap &schedule);

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

    void lockStatePushed(bool success, bool locked);

    void pushSuccess();
    void pushFailed();

    void installedSuccess();
    void installFailed(QString err, bool needToRetry);

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
    void warrantyReplacementFinished(bool success = false, QString error = QString(), bool needToRetry = false);

    void jobInformationReady(bool success, QVariantMap data, QString error, bool needToRetry = false);

    void zipCodeInfoReady(bool success, QVariantMap data, bool needToRetry = false);
    void customerInfoReady(bool success, QVariantMap data,  QString error, bool needToRetry = false);

    void outdoorTemperatureReady(bool success = false, double temp = -1.0);

    void scheduleCleared(int id, bool success);
    void scheduleEdited(int id, bool success);
    void scheduleAdded(QString scheduleUid, bool success, QVariantMap schedule = QVariantMap());

private slots:
    //! Check firmware update with getSettings reply
    void checkFirmwareUpdate(QJsonObject settings);

private:
    QByteArray preparePacket(QString className, QString method, QJsonArray params);

private:
    bool mHasClient;
    QString mSerialNumber;
    QDateTime mLastPushTime;
    QDateTime mAutoModeLastPushTime;
    QVariantMap mContractorInfo;
    cpuid_t mSystemUuid;

#ifdef SERIAL_TEST_MODE_ON
    int mSerialTestDelayCounter{SERIAL_TEST_DELAY_COUNTER};
#endif
};
}

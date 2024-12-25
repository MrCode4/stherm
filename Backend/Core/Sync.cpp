#include "Sync.h"
#include "LogHelper.h"
#include "Config.h"
#include "DeviceInfo.h"

#include <QImage>
#include <QUrl>

#include "LogHelper.h"
#include "device_config.h"

Q_LOGGING_CATEGORY(SyncLogCat, "SyncLog")
#define SYNC_LOG TRACE_CATEGORY(SyncLogCat)

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
namespace NUVE {
const QString cSerialNumberSetting = QString("NUVE/SerialNumber");
const QString cHasClientSetting = QString("NUVE/SerialNumberClient");
const QString cContractorSettings = QString("NUVE/Contractor");
const QString cWarrantySerialNumberKey = QString("NUVE/WarrantySerialNumber");
const QString cFirmwareUpdateKey      = QString("firmware");
const QString cFirmwareImageKey       = QString("firmware-image");
const QString cFirmwareForceUpdateKey = QString("force-update");

inline QDateTime updateTimeStringToTime(const QString &timeStr) {

    QString format = "yyyy-MM-dd HH:mm:ss";

    QDateTime dateTimeObject = QDateTime::fromString(timeStr, format);
    // explicitly set as UTC for better compare
    dateTimeObject.setTimeZone(QTimeZone(0));

    return dateTimeObject;
}


Sync::Sync(QObject *parent)
    : DevApiExecutor(parent)
    , mHasClient(false)
{
    QSettings setting;

    mSerialNumber = setting.value(cSerialNumberSetting).toString();
    mHasClient = setting.value(cHasClientSetting).toBool();

#if defined(FAKE_UID_MODE_ON) || defined(SERIAL_TEST_MODE_ON)
    mSerialNumber = "";
    mHasClient = false;
#endif

#if defined(INITIAL_SETUP_MODE_ON)
    mHasClient = false;
#endif

    mContractorInfo = setting.value(cContractorSettings).toMap();

    connect(this, &Sync::contractorInfoReady, this, [this]() {
        QSettings setting;
        setting.setValue(cContractorSettings, mContractorInfo);
    });
}

void Sync::setSerialNumber(const QString &serialNumber)
{
    if (serialNumber.isEmpty() || serialNumber == mSerialNumber){
        TRACE << "serial number not set:" << serialNumber << ", current is :" << mSerialNumber
              << ", has client is :" << mHasClient << ", set to true";
        mHasClient            = true;
        return;
    }

    mHasClient = true;
    // Update SN for get settings
    mSerialNumber = serialNumber;
    QSettings setting;
    setting.setValue(cHasClientSetting, mHasClient);
    setting.setValue(cSerialNumberSetting, mSerialNumber);

    // Force to update with new settings
    mLastPushTime = QDateTime();
    mAutoModeLastPushTime = QDateTime();
    // Fetch with new serial number
    emit serialNumberChanged();

    Device->updateSerialNumber(serialNumber, true);
}

void Sync::setUID(cpuid_t accessUid)
{
    mSystemUuid = accessUid;
    Device->uid(accessUid);
}

QString Sync::getSerialNumber() const { return mSerialNumber;}
bool Sync::hasClient() const { return mHasClient; }
QVariantMap Sync::getContractorInfo() const { return mContractorInfo; }

void Sync::fetchSerialNumber(const QString& uid, bool notifyUser)
{
    QPointer<QEventLoop> eventLoop;

    auto callback = [this, &eventLoop, notifyUser](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (data.contains("serial_number")) {
            auto sn = data.value("serial_number").toString();

#ifdef INITIAL_SETUP_MODE_ON
            mHasClient = false;
#elif defined(SERIAL_TEST_MODE_ON)
            if (mSerialTestDelayCounter > 0) {
                mHasClient = false;
                sn = "";
                --mSerialTestDelayCounter;
            } else {
                mHasClient = data.value("has_client").toBool();
            }
#else
            mHasClient = data.value("has_client").toBool();
#endif

            TRACE << sn << mHasClient;

            if (!mHasClient) {
                TRACE << "will start initial setup!";
            }

            if (!mSerialNumber.isEmpty() && sn != mSerialNumber) {
                emit alert("The serial number does not match the last one.");

                if (!mHasClient) {
                    // Update SN for get settings
                    mSerialNumber = sn;
                    // Force to update with new settings
                    mLastPushTime = QDateTime();
                    // Fetch with new serial number
                    emit serialNumberChanged();

                    mAutoModeLastPushTime = QDateTime();
                }

                TRACE << "The serial number does not match the last one." << mSerialNumber << sn;
            }
            else if (sn.isEmpty()) {
                emit alert("Oops...\nlooks like this device is not recognized by "
                           "our servers,\nplease send it to the manufacturer and\n "
                           "try to install another device.");
            }

            mSerialNumber = sn;

            if (!mSerialNumber.isEmpty()) {
                emit serialNumberReady();
            }

            if (mSerialNumber.isEmpty() && notifyUser) {
                emit testModeStarted();
            }

            // Save the serial number in settings
#if !defined(FAKE_UID_MODE_ON) && !defined(INITIAL_SETUP_MODE_ON) && !defined(SERIAL_TEST_MODE_ON)
            QSettings setting;
            setting.setValue(cHasClientSetting, mHasClient);
            setting.setValue(cSerialNumberSetting, mSerialNumber);
#endif
            Device->updateSerialNumber(mSerialNumber, mHasClient);
        }
        else {
            if (reply->error() == QNetworkReply::NoError) {
                TRACE << "No serial number has returned by server";
            }
            else if (notifyUser && reply->error() == QNetworkReply::ContentNotFoundError) {
                emit testModeStarted();
            }
            else {
                qWarning() << "Unable to fetch the device serial number, error: " << reply->errorString();

                if (!mSerialNumber.isEmpty()) {
                    TRACE << "Serial number has error but was filled previously. " << mSerialNumber;
                    emit serialNumberReady();
                }
            }
        }

        if (eventLoop && eventLoop->isRunning()) {
            eventLoop->quit();
        }
    };

    auto netReply = callGetApi(baseUrl() + QString("api/sync/getSn?uid=%0").arg(uid), callback, false);

    if (netReply) {
        // block if the first serial is invalid or client is not set yet
        if (mSerialNumber.isEmpty() || !mHasClient) {
            QEventLoop loop;
            eventLoop = &loop;
            loop.exec();

            eventLoop.clear();  // Clear the pointer after exec completes
        }
    }
}

bool Sync::fetchContractorInfo()
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "ContractorInfo: The serial number is not recognized correctly...";
        return false;
    }

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() != QNetworkReply::NoError) {
            emit contractorInfoReady(false);
        }
        else {
            auto brandValue = data.value("brand");
            auto phoneValue = data.value("phone");
            auto logoValue = data.value("logo");

            if (data.isEmpty() || !brandValue.isString() || brandValue.toString().isEmpty()) {
                TRACE << "Wrong contractor info fetched from server";
                emit contractorInfoReady(false);
                return;
            }

            QVariantMap map;
            map.insert("phone", phoneValue.toString(mContractorInfo.value("phone").toString()));
            map.insert("brand", brandValue.toString(mContractorInfo.value("brand").toString()));
            map.insert("url", data.value("url").toString(mContractorInfo.value("url").toString()));
            map.insert("tech", data.value("schedule").toString(mContractorInfo.value("tech").toString()));
            // logo is a bit more complicated than others,
            // the value inside map should be either empty so it loads from brand
            // name, or be a resource or local fs path so if it has the logo
            // response we keep it empty until the actual value handled (if not
            // empty, it should be downloaded to a local path).
            map.insert("logo", "");
            mContractorInfo = map;

            auto logo = logoValue.toString();
            if (logo.isEmpty()) {
                emit contractorInfoReady();
            }
            else {
                fetchContractorLogo(logo);
            }

            fetchUserData();
        }
    };

    return callGetApi(baseUrl() + QString("api/sync/getContractorInfo?sn=%0").arg(mSerialNumber), callback) != nullptr;
}

void Sync::fetchContractorLogo(const QString &url)
{
    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            QImage image;
            if (image.loadFromData(rawData)) {
                // Use 'usr' directory in the windows. It will be change in unix.
                QString imgPath = "/usr/local/customIcon.png";
#ifdef unix
                imgPath = "/home/root/customIcon.png";
#endif
                if (image.save(imgPath)) {
                    mContractorInfo.insert("logo", "file://" + imgPath);

                } else {
                    qWarning() << "Contractor logo could not be saved. " << imgPath << image.isNull();
                }
            }
        }

        emit contractorInfoReady();
    };

    downloadFile(url, callback, false);
}

void Sync::checkFirmwareUpdate(QJsonObject settings)
{
    QString fwVersion;

    if (settings.contains(cFirmwareUpdateKey) &&
        settings.value(cFirmwareUpdateKey).isObject()) {
        auto fwUpdateObj = settings.value(cFirmwareUpdateKey).toObject();
        auto fwUpdateVersion = fwUpdateObj.value(cFirmwareImageKey).toString("");

        // if force-update is set to true, then firmware-image instructs device to update to that version
        if (fwUpdateObj.value(cFirmwareForceUpdateKey).toBool()) {
            fwVersion = fwUpdateVersion;
        }
    }

    emit updateFirmwareFromServer(fwVersion);
}


bool Sync::fetchSettings()
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get settings!";
        // false preventing the fetch timer to be disabled
        return false;
    }

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        bool success = false;
        if (data.isEmpty()) {
            if (reply->error() == QNetworkReply::NoError) {
                TRACE << "Received settings corrupted: " + mSerialNumber;
            }
        }
        else if (data.value("sn").toString() == mSerialNumber) {
            auto dateString = data.value("setting").toObject().value("last_update");
            TRACE << "cpp last_update:" << dateString;
            QDateTime dateTimeObject = updateTimeStringToTime(dateString.toString());

            if (dateTimeObject.isValid()) {
                // Use the dateTimeObject here with time information
                TRACE << "Date with time cpp set last_update: " << dateTimeObject
                      << dateTimeObject.toString() << (mLastPushTime > dateTimeObject)
                      << (mLastPushTime == dateTimeObject)
                      << (mLastPushTime < dateTimeObject);
            }
            else {
                TRACE << "Invalid date format! cpp set last_update:";
            }

            if (!mLastPushTime.isNull() && (!dateTimeObject.isValid() || mLastPushTime >= dateTimeObject)) {
                TRACE << "Received settings has invalid date last_update: " + dateTimeObject.toString();
            }
            else {
                mLastPushTime = dateTimeObject;
                emit settingsReady(data.toVariantMap());
            }

            success = true;

            // Transmit Non-Configuration Data to UI and Update Model on Server
            // Response
            emit appDataReady(data.toVariantMap());
            checkFirmwareUpdate(data);
        }
        else {
            TRACE << "Received settings belong to another device: " + mSerialNumber + ", " + data.value("sn").toString();
        }

        // emits settingsFetched to allow next fetch
        fetchAutoModeSetings(success);
    };

    auto reply = callGetApi(baseUrl() + QString("api/sync/getSettings?sn=%0").arg(mSerialNumber), callback);
    if (reply) {
        reply->setProperty("noContentLog", true);
    }

    return reply != nullptr;
}

bool Sync::fetchAutoModeSetings(bool success)
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get auto mode settings!";
        // to preserve he flow! although this must not happen!
        emit settingsFetched(false);
        return false;
    }

    auto callback = [this, success](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (data.isEmpty()) {
            TRACE << "Received settings corrupted";
        }

        QDateTime dateTimeObject = updateTimeStringToTime(data.value("last_update").toString());
        if (dateTimeObject.isValid()) {
            // Use the dateTimeObject here with time information
            TRACE << "Auto mode: date with time cpp set last_update: " << dateTimeObject
                  << dateTimeObject.toString() << (mAutoModeLastPushTime > dateTimeObject)
                  << (mAutoModeLastPushTime == dateTimeObject)
                  << (mAutoModeLastPushTime < dateTimeObject);
        } else {
            TRACE << "Auto mode: Invalid date format! cpp set last_update:";
        }

        // check date time
        if (!mAutoModeLastPushTime.isNull() && (!dateTimeObject.isValid() || mAutoModeLastPushTime >= dateTimeObject)) {
            TRACE << "Auto mode: Received auto mode settings has invalid date last_update: " + dateTimeObject.toString();
        } else {
            mAutoModeLastPushTime = dateTimeObject;
            emit autoModeSettingsReady(data.toVariantMap(), !data.isEmpty());
        }

        // what if auto mode sucess but normal not!
        emit settingsFetched(success && !data.isEmpty());
    };

    return callGetApi(baseUrl() + QString("api/sync/autoMode?sn=%0").arg(mSerialNumber), callback) != nullptr;
}

bool Sync::fetchMessages()
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get messages!";
        return false;
    }

    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &) {
        emit messagesLoaded();
    };

    return callGetApi(baseUrl() + QString("api/sync/messages?sn=%0").arg(mSerialNumber), callback) != nullptr;
}

void Sync::fetchWirings(const QString& uid)
{
    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &) {
        emit wiringReady();
    };

    callGetApi(baseUrl() + QString("api/sync/getWirings?uid=%0").arg(uid), callback);
}

void Sync::fetchUserData()
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get user-data!";
        return;
    }

    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &data) {
        if (data.isEmpty()) {
            TRACE << "Received user-data corrupted";
        }
        else {
            emit userDataFetched(data.value("email").toString(), data.value("name").toString());
        }

        fetchingUserData(false);
    };

    fetchingUserData(true);
    callGetApi(baseUrl() + QString("api/sync/client?sn=%0").arg(mSerialNumber), callback);
}

void Sync::pushLockState(const QString& pin, bool lock)
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not update lock status!";
        return;
    }

    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &data) {
        emit lockStatePushed(data.contains("locked"), data.value("locked").toBool());
        pushingLockState(false);
    };

    TRACE << "Pushing device lock state changes: " << lock;
    pushingLockState(true);

    auto endpoint = QString("api/sync/screen-%1?sn=%2").arg(lock ? "lock" : "unlock").arg(mSerialNumber);
    QJsonObject requestBody;
    requestBody["pin"] = pin;
    callPostApi(baseUrl() + endpoint, QJsonDocument(requestBody).toJson(), callback);
}

QByteArray Sync::preparePacket(QString className, QString method, QJsonArray params)
{
    QJsonObject requestData;
    requestData["request"] =
        QJsonObject{{"class", className}, {"method", method}, {"params", params}};

    requestData["user"] =
        QJsonObject{{"lang_id", 0}, {"user_id", 0},   {"type_id", 0},
                    {"host_id", 0}, {"region_id", 0}, {"token", ""}};

    QJsonDocument jsonDocument(requestData);

    return jsonDocument.toJson();
}

void Sync::requestJob(QString type)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(type);
    callPostApi(baseUrl() + "engine/index.php", preparePacket("sync", "requestJob", paramsArray));
}

void Sync::pushSettingsToServer(const QVariantMap &settings)
{
    QJsonObject reqData = QJsonObject::fromVariantMap(settings);
    reqData["sn"] = mSerialNumber;

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            // get the last update
            auto dateString = data.value("setting").toObject().value("last_update");
            TRACE << "cpp set last_update:" << dateString;
            QDateTime dateTimeObject = updateTimeStringToTime(dateString.toString());

            if (dateTimeObject.isValid()) {
                // Use the dateTimeObject here with time information
                TRACE << "Date with time cpp set last_update: " << dateTimeObject << dateTimeObject.toString();
                mLastPushTime = dateTimeObject;
            } else {
                TRACE << "Invalid date format! cpp set last_update:";
            }

            emit pushSuccess();
        }
        else {
            emit pushFailed();
        }
    };

    callPostApi(baseUrl() + "api/sync/update", QJsonDocument(reqData).toJson(), callback);
}

void Sync::pushAutoSettingsToServer(const double &auto_temp_low, const double &auto_temp_high)
{
    QJsonObject reqData;
    reqData["auto_temp_low"] = auto_temp_low;
    reqData["auto_temp_high"] = auto_temp_high;
    // Temporary, has no effect
    reqData["mode"] = "auto";
    reqData["is_active"] = true;

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            TRACE << "Auto mode settings pushed to server: setAutoModeSettings";

            auto dateString = data.value("last_update");
            QDateTime dateTimeObject =  QDateTime::fromString(dateString.toString(), Qt::ISODate);
            dateTimeObject.setTimeZone(QTimeZone(0)); // explicitly set as UTC for better compare

            if (dateTimeObject.isValid()) {
                // Use the dateTimeObject here with time information
                TRACE << "Auto mode:  Date with time cpp set last_update: " << dateTimeObject << dateTimeObject.toString();
                mAutoModeLastPushTime = dateTimeObject;

            } else {
                TRACE << "Auto mode: Invalid date format! cpp set last_update:";
            }

            emit autoModePush(true);
        } else {
            emit autoModePush(false);
        }
    };

    callPostApi(baseUrl() + QString("api/sync/autoMode?sn=%0").arg(mSerialNumber), QJsonDocument(reqData).toJson(), callback);
}

void Sync::fetchServiceTitanInformation()
{
    // TODO: Get service titan data with proper API
    // We need email, ZIP code and isServiceTitanActive properties
    // We can get customers information such as Email, Phone number, Zip code and
    // address BUT only Email and ZIP code need to be reflected

    QPointer<QEventLoop> eventLoop;

    auto callback = [this, &eventLoop](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
    };

    QNetworkReply* netReply = nullptr;

    if (netReply) {
        QEventLoop loop;
        eventLoop = &loop;
        loop.exec();

        eventLoop.clear();
    } else {
        emit serviceTitanInformationReady(false);
    }
}

void Sync::getJobIdInformation(const QString& jobID)
{
    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        QString err = getReplyError(reply);

        TRACE_CHECK(reply->error() != QNetworkReply::NoError) << "Job Information error: " << err;

        auto isNeedRetry = isNeedRetryNetRequest(reply);
        emit jobInformationReady(!data.isEmpty(), data.toVariantMap(), err, data.isEmpty() && reply->error() != QNetworkReply::UnknownContentError);
    };

    auto netReply =  callGetApi(baseUrl() + QString("/api/technicians/service-titan/customer/%0?sn=%1").arg(jobID, mSerialNumber), callback);
    if (!netReply) {
        TRACE << "call get api canceled for getJobIdInformation";
        //        emit jobInformationReady(false, QVariantMap());
    }
}

void Sync::getCustomerInformationManual(const QString &email)
{
    auto callbackCustomer = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        TRACE_CHECK(reply->error() != QNetworkReply::NoError) << "Job Information error: " << reply->errorString();

        auto err = getReplyError(reply);

        // data can be empty when email is new
        auto isNeedRetry = isNeedRetryNetRequest(reply);
        emit customerInfoReady(reply->error() == QNetworkReply::NoError, data.toVariantMap(), err, isNeedRetry);
    };

    auto api = prepareUrlWithEmail(baseUrl() + QString("/api/customer"), email);
    auto netReply =  callGetApi(api, callbackCustomer);
    if (!netReply) {
        TRACE << "call get api canceled for customer";
        //        emit customerInfoReady(false, QVariantMap());
    }
}

void Sync::getAddressInformationManual(const QString &zipCode)
{
    auto callbackZip = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        TRACE_CHECK(reply->error() != QNetworkReply::NoError) << "Address Information error: " << reply->errorString();

        auto isNeedRetry = isNeedRetryNetRequest(reply);
        emit zipCodeInfoReady(!data.isEmpty(), data.toVariantMap(), isNeedRetry);
    };

    auto netReply =  callGetApi(baseUrl() + QString("/api/zipCode?code=%0").arg(zipCode), callbackZip);
    if (!netReply) {
        TRACE << "call get api canceled for zip";
        //        emit zipCodeInfoReady(false, QVariantMap());
    }
}

void Sync::installDevice(const QVariantMap &data)
{
    QJsonObject reqData = QJsonObject::fromVariantMap(data);

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {

            auto enabled = data.value("is_enabled");
            TRACE << rawData << data << enabled;
            emit installedSuccess();
        }
        else {

            auto err = getReplyError(reply);

            auto isNeedRetry = isNeedRetryNetRequest(reply);
            emit installFailed(err, isNeedRetry);
            TRACE << rawData << data << err;
        }
    };

    TRACE << QJsonDocument(reqData).toJson();
    callPostApi(baseUrl() + "/api/technicians/device/install", QJsonDocument(reqData).toJson(), callback);
}

void Sync::warrantyReplacement(const QString &oldSN, const QString &newSN)
{
    // TODO: Warranty replacement implementation
    if (oldSN != newSN) {
        QSettings setting;
        setting.setValue(cWarrantySerialNumberKey, oldSN);

        auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
            QString err;
            bool success = false;
            if (reply->error() == QNetworkReply::NoError) {
                // get the last update
                auto message = data.value("message").toString();
                TRACE << "warrantyReplacement message:" << message;
                success = true;

            } else {
                err = getReplyError(reply);
            }

            emit warrantyReplacementFinished(success, err, reply->error() != QNetworkReply::UnknownContentError);
        };

        QJsonObject reqData;
        reqData["old_sn"] = oldSN;
        reqData["new_sn"] = newSN;

        auto netReply =  callPostApi(baseUrl() + QString("/api/technicians/warranty"), QJsonDocument(reqData).toJson(), callback);
        if (!netReply) {
            TRACE << "call get api canceled for warranty";
            //            emit warrantyReplacementFinished(false);
        }
    } else {
        emit warrantyReplacementFinished(false, QString("The old and new serial numbers are the same."));
    }
}

void Sync::pushAlertToServer(const QVariantMap &settings)
{
    QJsonObject requestDataObj;
    requestDataObj["sn"] = mSerialNumber;
    QJsonObject requestDataObjAlert;
    requestDataObjAlert["alert_id"] = 1;
    QJsonArray requestDataObjAlertArr;
    requestDataObjAlertArr.append(requestDataObjAlert);
    requestDataObj["alerts"] = requestDataObjAlertArr;
    callPostApi(baseUrl() + "api/sync/alerts", QJsonDocument(requestDataObj).toJson());
}

void Sync::forgetDevice()
{
    mHasClient = false;
    mSerialNumber = QString();
    mContractorInfo = QVariantMap{};

    // Save the serial number in settings
    QSettings setting;
    setting.setValue(cHasClientSetting, mHasClient);
    setting.setValue(cSerialNumberSetting, mSerialNumber);
    setting.setValue(cContractorSettings, mContractorInfo);
    Device->reset();
}

void Sync::clearSchedule(const int &scheduleID)
{
    auto callback = [this, scheduleID](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        bool success = reply->error() == QNetworkReply::NoError;

        if (success) {
            success = data.value("errors").isUndefined() || data.value("errors").toArray().empty();
        }

        if (!success) {
            TRACE << "clearSchedule" << data.value("errors") << data.value("message") << reply->errorString();
        }

        emit scheduleCleared(scheduleID, success);
    };

    auto reply = callPostApi(baseUrl() + QString("api/sync/clearSchedules?sn=%0&id=%1").arg(mSerialNumber, QString::number(scheduleID)), QJsonDocument().toJson(), callback);
    if (reply) {// returned response has no data object and values are in root
        reply->setProperty("noDataObject", true);
    }
}

void Sync::editSchedule(const int &scheduleID, const QVariantMap &schedule)
{
    QJsonObject reqData = QJsonObject::fromVariantMap(schedule);
    reqData["sn"] = mSerialNumber;

    auto callback = [this, scheduleID](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            emit scheduleEdited(scheduleID, true);
        }
        else {
            TRACE << "editSchedule" << reply->errorString();
            emit scheduleEdited(scheduleID, false);
        }
    };

    auto reply = callPutApi(baseUrl() + QString("api/sync/schedules/%0").arg(QString::number(scheduleID)), QJsonDocument(reqData).toJson(), callback);
    if (reply) {// returned response has no data object and values are in root
        reply->setProperty("noDataObject", true);
    }
}

void Sync::addSchedule(const QString &scheduleUid, const QVariantMap &schedule)
{
    QJsonObject reqData = QJsonObject::fromVariantMap(schedule);
    reqData["sn"] = mSerialNumber;

    auto callback = [this, scheduleUid](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            emit scheduleAdded(scheduleUid, !data.isEmpty(), data.toVariantMap());
        }
        else {
            TRACE << "addSchedule" << reply->errorString();
            emit scheduleAdded(scheduleUid, false);
        }
    };

    auto reply = callPostApi(baseUrl() + QString("api/sync/schedules"), QJsonDocument(reqData).toJson(), callback);
    if (reply) {// returned response has no data object and values are in root
        reply->setProperty("noDataObject", true);
    }
}

void Sync::resetFactory()
{
    // Forget request is unnecessary if the serial number is empty or, when the serial number is valid, if hasClient is false
    // but the 2nd case is not important.
    if (mSerialNumber.isEmpty()) {
        emit resetFactorySucceeded();
        return;
    }

    QJsonObject reqData;
    reqData["sn"] = mSerialNumber;

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            emit resetFactorySucceeded();
        } else {
            TRACE << "resetFactory" << reply->errorString();
            emit resetFactoryFailed(reply->errorString());
        }
    };

    auto reply = callPostApi(baseUrl() + QString("api/sync/forget?sn=%1").arg(mSerialNumber),
                             QJsonDocument(reqData).toJson(),
                             callback);
    if (reply) { // returned response has no data object and values are in root
        reply->setProperty("noDataObject", true);
    }
}

void Sync::getOutdoorTemperature() {

    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get outdoor temperature!";
        return;
    }

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            auto var = data.value("value");
            // what is the best validation?
            emit outdoorTemperatureReady(!var.isUndefined(), var.toDouble());
        }
        else {
            emit  outdoorTemperatureReady();
        }
    };

    auto reply = callGetApi(baseUrl() + QString("/api/weather?sn=%0").arg(mSerialNumber), callback);
    if (reply) {// returned response has no data object and values are in root
        reply->setProperty("noDataObject", true);
    }
}

void Sync::reportCommandResponse(ReportCommandCallback callback, const QString& command, const QString& response, int retryCount)
{
    if (mSerialNumber.isEmpty()) {
        SYNC_LOG <<"Sn is not ready! can not report command" << command;
        if (callback != nullptr) {
            callback(false, QJsonObject());
        }
        return;
    }

    auto apiCallback = [=] (QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() != QNetworkReply::NoError && retryCount > 0) {
            SYNC_LOG << "Reporting Command failed, will retry in 1 minute. Retry left" << retryCount - 1;
            QTimer::singleShot(60 * 1000, this, [=]() {
                SYNC_LOG << "Reporting Command retring";
                reportCommandResponse(callback, command, response, retryCount - 1);
            });
        }
        else if (callback != nullptr) {
            SYNC_LOG << "Reporting command" <<command <<"success";
            callback(reply->error() == QNetworkReply::NoError, data);
        }
    };

    QJsonObject body;
    body["data"] = response;
    SYNC_LOG <<"Reporting command" <<command << "with data" <<response;
    callPostApi(baseUrl() + QString("api/monitor/report?sn=%0").arg(mSerialNumber), QJsonDocument(body).toJson(), apiCallback);
}

} // namespace NUVE

#include "Sync.h"
#include "LogHelper.h"

#include <QImage>
#include <QUrl>

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
namespace NUVE {
const QString cBaseUrl = "https://devapi.nuvehvac.com/"; // base domain
const QString cSerialNumberSetting = QString("NUVE/SerialNumber");
const QString cHasClientSetting = QString("NUVE/SerialNumberClient");
const QString cContractorSettings = QString("NUVE/Contractor");
const QString cFirmwareUpdateKey      = QString("firmware");
const QString cFirmwareImageKey       = QString("firmware-image");
const QString cFirmwareForceUpdateKey = QString("force-update");

inline QDateTime updateTimeStringToTime(const QString &timeStr) {

    QString format = "yyyy-MM-dd HH:mm:ss";

    return QDateTime::fromString(timeStr, format);
}


Sync::Sync(QObject *parent)
    : RestApiExecutor(parent)
    , mHasClient(false)
    , m_fetchingUserData(false)
{
    QSettings setting;
    mHasClient = setting.value(cHasClientSetting).toBool();
    mSerialNumber = setting.value(cSerialNumberSetting).toString();
    mContractorInfo = setting.value(cContractorSettings).toMap();

    connect(this, &Sync::contractorInfoReady, this, [this]() {
        QSettings setting;
        setting.setValue(cContractorSettings, mContractorInfo);
    });
}

void Sync::setSerialNumber(const QString &serialNumber)
{
    if (serialNumber.isEmpty() || serialNumber == mSerialNumber){
        TRACE << "serial number not set:" << serialNumber << ", current is :" << mSerialNumber;
        return;
    }

    mHasClient            = true;
    // Update SN for get settings
    mSerialNumber         = serialNumber;
    // Force to update with new settings
    mLastPushTime = QDateTime();
    mAutoModeLastPushTime = QDateTime();
    // Fetch with new serial number
    emit serialNumberChanged();
}

void Sync::setUID(cpuid_t accessUid) { mSystemUuid = accessUid; }
QString Sync::getSerialNumber() const { return mSerialNumber;}
bool Sync::hasClient() const { return mHasClient; }
QVariantMap Sync::getContractorInfo() const { return mContractorInfo; }

void Sync::setApiAuth(QNetworkRequest& request)
{
    RestApiExecutor::setApiAuth(request);
    auto authData = mSystemUuid + mSerialNumber.toStdString();
    // Get error: QNetworkReply::ProtocolFailure "Server is unable to maintain
    // the header compression context for the connection"
    request.setRawHeader("Authorization", "Bearer " + QCryptographicHash::hash(authData, QCryptographicHash::Sha256).toHex());
}

QJsonObject Sync::prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const
{
    QJsonObject data;
    const QJsonObject rootObject = RestApiExecutor::prepareJsonResponse(endpoint, rawData);

    if (rootObject.contains("data")) {
        data = rootObject.value("data").toObject();
    }
    else {
        TRACE << "API ERROR (" << endpoint << ") : " << " Reponse contains no data object";
    }

    return data;
}


void Sync::fetchSerialNumber(const QString& uid, bool notifyUser)
{
    QEventLoop* eventLoop = nullptr;

    auto callback = [this, &eventLoop, notifyUser](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (data.contains("serial_number")) {
            auto sn = data.value("serial_number").toString();
            mHasClient = data.value("has_client").toBool();
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
            QSettings setting;
            setting.setValue(cHasClientSetting, mHasClient);
            setting.setValue(cSerialNumberSetting, mSerialNumber);
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
            }
        }

        if (eventLoop) {
            eventLoop->quit();
        }
    };

    auto netReply = callGetApi(cBaseUrl + QString("api/sync/getSn?uid=%0").arg(uid), callback, false);

    if (netReply) {
        // block if the first serial is invalid or client is not set yet
        if (mSerialNumber.isEmpty() || !mHasClient) {
            QEventLoop loop;
            eventLoop = &loop;
            loop.exec();
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
            emit contractorInfoReady();
        }
        else {
            auto brandValue = data.value("brand");
            auto phoneValue = data.value("phone");
            auto logoValue = data.value("logo");

            if (data.isEmpty() || !brandValue.isString() || brandValue.toString().isEmpty()) {
                TRACE << "Wrong contractor info fetched from server";
                emit contractorInfoReady();
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
            // empty, it should be downloaded to a local path) and if it has null
            // or has not value, we will keep the previous value
            map.insert("logo", logoValue.isString() ? "" : mContractorInfo.value("logo").toString());
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

    return callGetApi(cBaseUrl + QString("api/sync/getContractorInfo?sn=%0").arg(mSerialNumber), callback) != nullptr;
}

void Sync::fetchContractorLogo(const QString &url)
{
    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            QImage image;
            if (image.loadFromData(rawData)) {
                QString imgPath = "/usr/local/customIcon.png";
#ifdef unix
                imgPath = "/home/root/customIcon.png";
#endif
                if (!image.save(imgPath)) {
                    qWarning() << "Contractor logo could not be saved. " << imgPath << image.isNull();
                }
                mContractorInfo.insert("logo", "file://" + imgPath);
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
        bool success = true;
        if (data.isEmpty()) {
            if (reply->error() == QNetworkReply::NoError) {
                TRACE << "Received settings corrupted: " + mSerialNumber;
                success = false;
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

            // Transmit Non-Configuration Data to UI and Update Model on Server
            // Response
            emit appDataReady(data.toVariantMap());
            checkFirmwareUpdate(data);
        }
        else {
            success = false;
            TRACE << "Received settings belong to another device: " + mSerialNumber + ", " + data.value("sn").toString();
        }

        // emits settingsFetched to allow next fetch
        fetchAutoModeSetings(success);
    };

    auto reply = callGetApi(cBaseUrl + QString("api/sync/getSettings?sn=%0").arg(mSerialNumber), callback);
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

    return callGetApi(cBaseUrl + QString("api/sync/autoMode?sn=%0").arg(mSerialNumber), callback) != nullptr;
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

    return callGetApi(cBaseUrl + QString("api/sync/messages?sn=%0").arg(mSerialNumber), callback) != nullptr;
}

void Sync::fetchWirings(const QString& uid)
{
    auto callback = [this](QNetworkReply *, const QByteArray &, QJsonObject &) {
        emit wiringReady();
    };

    callGetApi(cBaseUrl + QString("api/sync/getWirings?uid=%0").arg(uid), callback);
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
    callGetApi(cBaseUrl + QString("api/sync/client?sn=%0").arg(mSerialNumber), callback);
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
    callPostApi(cBaseUrl + "engine/index.php", preparePacket("sync", "requestJob", paramsArray));
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

    callPostApi(cBaseUrl + "api/sync/update", QJsonDocument(reqData).toJson(), callback);
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

    callPostApi(cBaseUrl + QString("api/sync/autoMode?sn=%0").arg(mSerialNumber), QJsonDocument(reqData).toJson(), callback);
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
    callPostApi(cBaseUrl + "api/sync/alerts", QJsonDocument(requestDataObj).toJson());
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
}

} // namespace NUVE

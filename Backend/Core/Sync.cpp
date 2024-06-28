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
const char *cNotifyGetSN = "notifyGetSN";

inline QDateTime updateTimeStringToTime(const QString &timeStr) {
    QString format = "yyyy-MM-dd HH:mm:ss";

    return QDateTime::fromString(timeStr, format);
}

Sync::Sync(QObject *parent)
    : NetworkWorker(parent)
    , mHasClient(false)
{
    QSettings setting;
    mHasClient = setting.value(cHasClientSetting).toBool();
    mSerialNumber = setting.value(cSerialNumberSetting).toString();
    mContractorInfo = setting.value(cContractorSettings).toMap();

    connect(this, &Sync::contractorInfoReady, [this]() {
        QSettings setting;
        setting.setValue(cContractorSettings, mContractorInfo);
    });
}

void Sync::setUID(cpuid_t accessUid) { mSystemUuid = accessUid; }

QNetworkRequest Sync::prepareApiRequest(const QString &endpoint, bool setAuth) {
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("accept", "application/json");
    request.setTransferTimeout(8000);

    if (setAuth) {
        auto authData = mSystemUuid + mSerialNumber.toStdString();
        // Get error: QNetworkReply::ProtocolFailure "Server is unable to maintain
        // the header compression context for the connection"
        request.setRawHeader("Authorization",
                             "Bearer " + QCryptographicHash::hash(
                                             authData, QCryptographicHash::Sha256)
                                             .toHex());
    }

    return request;
}

QNetworkReply *Sync::callGetApi(const QString &endpoint,
                                ResponseCallback callback, bool setAuth) {
    if (mCallbacks.contains(endpoint)) {
        return nullptr;
    } else {
        mCallbacks.insert(endpoint, callback);
        auto reply = get(prepareApiRequest(endpoint, setAuth));
        reply->setProperty("endpoint", endpoint);
        return reply;
    }
}

QNetworkReply *Sync::callPostApi(const QString &endpoint,
                                 const QByteArray &postData,
                                 ResponseCallback callback) {
    if (mCallbacks.contains(endpoint)) {
        return nullptr;
    } else {
        mCallbacks.insert(endpoint, callback);
        auto reply = post(prepareApiRequest(endpoint, true), postData);
        reply->setProperty("endpoint", endpoint);
        return reply;
    }
}

void Sync::processNetworkReply(QNetworkReply *reply) {
    auto endpoint = reply->property("endpoint").toString();
    if (!mCallbacks.contains(endpoint)) {
        TRACE << "Callback not found for endpoint " << endpoint;
        return;
    }

    QByteArray rawData;
    QJsonObject data;

    if (reply->error() != QNetworkReply::NoError) {
        TRACE << "API ERROR (" << reply->url().url()
              << ") : " << reply->errorString();
    } else {
        rawData = reply->readAll();
        if (reply->property("IsBinaryData").isValid() == false) {
            const QJsonObject jsonDocObj = QJsonDocument::fromJson(rawData).object();
            if (jsonDocObj.contains("data")) {
                data = jsonDocObj.value("data").toObject();
            } else {
                TRACE << "API ERROR (" << reply->url().url() << ") : "
                      << " Reponse contains no data object";
            }
        }
    }

    auto callback = mCallbacks.take(endpoint);
    if (callback) {
        callback(reply, rawData, data);
    }
}

QString Sync::getSerialNumber() const { return mSerialNumber; }

bool Sync::hasClient() const { return mHasClient; }

void Sync::fetchSerialNumber(cpuid_t accessUid, bool notifyUser) {
    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData,
                           QJsonObject &data) {
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
                }

                TRACE << "The serial number does not match the last one."
                      << mSerialNumber << sn;
            } else if (sn.isEmpty()) {
                emit alert("Oops...\nlooks like this device is not recognized by "
                           "our servers,\nplease send it to the manufacturer and\n "
                           "try to install another device.");
            }

            mSerialNumber = sn;

            auto notifyUser = reply->property(cNotifyGetSN).toBool();
            if (mSerialNumber.isEmpty() && notifyUser)
                emit testModeStarted();

            // Save the serial number in settings
            QSettings setting;
            setting.setValue(cHasClientSetting, mHasClient);
            setting.setValue(cSerialNumberSetting, mSerialNumber);
        } else {
            if (reply->error() == QNetworkReply::NoError) {
                TRACE << "No serial number has returned by server";
            } else {
                auto notifyUser = reply->property(cNotifyGetSN).toBool();
                if (notifyUser &&
                    reply->error() == QNetworkReply::ContentNotFoundError) {
                    emit testModeStarted();
                }
            }
        }

        emit serialNumberReady();
    };

    auto netReply = callGetApi(
        cBaseUrl + QString("api/sync/getSn?uid=%0").arg(accessUid.c_str()),
        callback, false);

    if (netReply) {
        netReply->setProperty(cNotifyGetSN, notifyUser);

        // block if the first serial is invalid or client is not set yet
        if (mSerialNumber.isEmpty() || !mHasClient) {
            QEventLoop loop;
            connect(this, &NUVE::Sync::serialNumberReady, &loop, [&loop]() { loop.quit(); });
            loop.exec(QEventLoop::ExcludeSocketNotifiers);
        }
    }
}

QVariantMap Sync::getContractorInfo() const { return mContractorInfo; }

void Sync::fetchContractorInfo() {
    if (mSerialNumber.isEmpty()) {
        qWarning()
            << "ContractorInfo: The serial number is not recognized correctly...";
        return;
    }

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData,
                           QJsonObject &data) {
        if (reply->error() != QNetworkReply::NoError) {
            emit contractorInfoReady();
        } else {
            auto brandValue = data.value("brand");
            auto phoneValue = data.value("phone");
            auto logoValue = data.value("logo");

            if (data.isEmpty() || !brandValue.isString() ||
                brandValue.toString().isEmpty()) {
                TRACE << "Wrong contractor info fetched from server";
                return;
            }

            QVariantMap map;
            map.insert("phone", phoneValue.toString(
                                    mContractorInfo.value("phone").toString()));
            map.insert("brand", brandValue.toString(
                                    mContractorInfo.value("brand").toString()));
            map.insert("url", data.value("url").toString(
                                  mContractorInfo.value("url").toString()));
            map.insert("tech",
                       data.value("schedule")
                           .toString(mContractorInfo.value("tech").toString()));
            // logo is a bit more complicated than others,
            // the value inside map should be either empty so it loads from brand
            // name, or be a resource or local fs path so if it has the logo
            // response we keep it empty until the actual value handled (if not
            // empty, it should be downloaded to a local path) and if it has null
            // or has not value, we will keep the previous value
            map.insert("logo", logoValue.isString()
                                   ? ""
                                   : mContractorInfo.value("logo").toString());
            mContractorInfo = map;

            auto logo = logoValue.toString();
            if (logo.isEmpty()) {
                emit contractorInfoReady();
            } else {
                fetchContractorLogo(logo);
            }
        }
    };

    callGetApi(cBaseUrl +
                   QString("api/sync/getContractorInfo?sn=%0").arg(mSerialNumber),
               callback);
}

void Sync::fetchContractorLogo(const QString &url) {
    // what if gets error, should we return immadiately?
    QNetworkRequest request(url);
    QNetworkReply *reply = get(request);
    reply->setProperty("IsBinaryData", true);
    reply->setProperty("endpoint", url);

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData,
                           QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            QImage image;
            if (image.loadFromData(rawData)) {
                image.save("/home/root/customIcon.png");
                mContractorInfo.insert("logo", "file:///home/root/customIcon.png");
            }
        }
        emit contractorInfoReady();
    };

    mCallbacks.insert(url, callback);
}

void Sync::fetchSettings() {
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get settings!";
        return;
    }

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData,
                           QJsonObject &data) {
        if (data.isEmpty()) {
            if (reply->error() == QNetworkReply::NoError) {
                TRACE << "Received settings corrupted: " + mSerialNumber;
            }
        } else {
            if (data.value("sn").toString() == mSerialNumber) {
                auto dateString = data.value("setting").toObject().value("last_update");
                TRACE << "cpp last_update:" << dateString;
                QDateTime dateTimeObject =
                    updateTimeStringToTime(dateString.toString());

                if (dateTimeObject.isValid()) {
                    // Use the dateTimeObject here with time information
                    TRACE << "Date with time cpp set last_update: " << dateTimeObject
                          << dateTimeObject.toString() << (mLastPushTime > dateTimeObject)
                          << (mLastPushTime == dateTimeObject)
                          << (mLastPushTime < dateTimeObject);
                } else {
                    TRACE << "Invalid date format! cpp set last_update:";
                }
                if (!mLastPushTime.isNull() &&
                    (!dateTimeObject.isValid() || mLastPushTime >= dateTimeObject)) {
                    TRACE << "Received settings has invalid date last_update: " +
                                 dateTimeObject.toString();

                } else {
                    mLastPushTime = dateTimeObject;
                    emit settingsReady(data.toVariantMap());
                }

                // Transmit Non-Configuration Data to UI and Update Model on Server
                // Response
                emit appDataReady(data.toVariantMap());
            } else {
                TRACE << "Received settings belong to another device: " +
                             mSerialNumber + ", " + data.value("sn").toString();
            }
        }

        fetchAutoModeSetings();
    };

    callGetApi(cBaseUrl + QString("api/sync/getSettings?sn=%0").arg(mSerialNumber), callback);
}

void Sync::fetchAutoModeSetings() {
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get auto mode settings!";
        return;
    }

    callGetApi(cBaseUrl + QString("api/sync/autoMode?sn=%0").arg(mSerialNumber),
               [this](QNetworkReply *reply, const QByteArray &rawData,
                      QJsonObject &data) {
                 emit autoModeSettingsReady(data.toVariantMap(),
                                            !data.isEmpty());
                 emit settingsFetched(!data.isEmpty());
               });
}

void Sync::getMessages() {
    if (mSerialNumber.isEmpty()) {
        qWarning() << "Sn is not ready! can not get messages!";
        return;
    }

    callGetApi(cBaseUrl + QString("api/sync/messages?sn=%0").arg(mSerialNumber),
               [this](QNetworkReply *reply, const QByteArray &rawData,
                      QJsonObject &data) { emit messagesLoaded(); });
}

void Sync::getWirings(cpuid_t accessUid) {
    callGetApi(cBaseUrl +
                   QString("api/sync/getWirings?uid=%0").arg(accessUid.c_str()),
               [this](QNetworkReply *, const QByteArray &, QJsonObject &) {
                   emit wiringReady();
               });
}

QByteArray Sync::preparePacket(QString className, QString method,
                               QJsonArray params) {
    QJsonObject requestData;
    requestData["request"] =
        QJsonObject{{"class", className}, {"method", method}, {"params", params}};

    requestData["user"] =
        QJsonObject{{"lang_id", 0}, {"user_id", 0},   {"type_id", 0},
                    {"host_id", 0}, {"region_id", 0}, {"token", ""}};

    QJsonDocument jsonDocument(requestData);

    return jsonDocument.toJson();
}

void Sync::requestJob(QString type) {
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(type);
    callPostApi(cBaseUrl + "engine/index.php",
                preparePacket("sync", "requestJob", paramsArray));
}

void Sync::pushSettingsToServer(const QVariantMap &settings) {
    QJsonObject reqData = QJsonObject::fromVariantMap(settings);
    reqData["sn"] = mSerialNumber;

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData,
                           QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            // get the last update
            auto dateString = data.value("setting").toObject().value("last_update");
            TRACE << "cpp set last_update:" << dateString;
            QDateTime dateTimeObject = updateTimeStringToTime(dateString.toString());

            if (dateTimeObject.isValid()) {
                // Use the dateTimeObject here with time information
                TRACE << "Date with time cpp set last_update: " << dateTimeObject
                      << dateTimeObject.toString();
                mLastPushTime = dateTimeObject;
            } else {
                TRACE << "Invalid date format! cpp set last_update:";
            }

            emit pushSuccess();
        } else {
            emit pushFailed();
        }
    };
    callPostApi(cBaseUrl + "api/sync/update", QJsonDocument(reqData).toJson(),
                callback);
}

void Sync::pushAutoSettingsToServer(const double &auto_temp_low,
                                    const double &auto_temp_high) {
    QJsonObject reqData;
    reqData["auto_temp_low"] = auto_temp_low;
    reqData["auto_temp_high"] = auto_temp_high;
    // Temporary, has no effect
    reqData["mode"] = "auto";
    reqData["is_active"] = true;

    auto callback = [this](QNetworkReply *reply, const QByteArray &rawData,
                           QJsonObject &data) {
        if (reply->error() == QNetworkReply::NoError) {
            TRACE << "Auto mode settings pushed to server: setAutoModeSettings";
            emit autoModePush(true);
        } else {
            emit autoModePush(false);
        }
    };

    callPostApi(cBaseUrl + QString("api/sync/autoMode?sn=%0").arg(mSerialNumber),
                QJsonDocument(reqData).toJson(), callback);
}

void Sync::pushAlertToServer(const QVariantMap &settings) {
    QJsonObject requestDataObj;
    requestDataObj["sn"] = mSerialNumber;
    QJsonObject requestDataObjAlert;
    requestDataObjAlert["alert_id"] = 1;
    QJsonArray requestDataObjAlertArr;
    requestDataObjAlertArr.append(requestDataObjAlert);
    requestDataObj["alerts"] = requestDataObjAlertArr;
    callPostApi(cBaseUrl + "api/sync/alerts",
                QJsonDocument(requestDataObj).toJson());
}

void Sync::ForgetDevice() {
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

#include "Sync.h"
#include "LogHelper.h"

#include <QImage>
#include <QUrl>

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
namespace NUVE {
const QUrl m_domainUrl        = QUrl("https://devapi.nuvehvac.com/"); // base domain
const QUrl m_engineUrl        = QUrl("/engine/index.php");          // engine
const QUrl m_updateUrl        = QUrl("/update/");                   // update
const QString m_getSN             = QString("getSN");
const QString m_getContractorInfo = QString("getContractorInfo");
const QString m_getContractorLogo = QString("getContractorLogo");
const QString m_getSettings = QString("getSettings");
const QString m_getAutoModeSettings = QString("getAutoModeSettings");
const QString m_getMessages = QString("getMessages");
const QString m_setSettings = QString("setSettings");
const QString m_setAutoModeSettings = QString("setAutoModeSettings");
const QString m_setAlerts = QString("setAlerts");
const QString m_getWirings = QString("getWirings");
const QString m_SerialNumberSetting = QString("NUVE/SerialNumber");
const QString m_HasClientSetting = QString("NUVE/SerialNumberClient");
const QString m_ContractorSettings = QString("NUVE/Contractor");
const QString m_requestJob      = QString("requestJob");

const QString m_firmwareUpdateKey      = QString("firmware");
const QString m_firmwareImageKey       = QString("firmware-image");
const QString m_firmwareForceUpdateKey = QString("force-update");

const char* m_notifyGetSN       = "notifyGetSN";

inline QDateTime updateTimeStringToTime(const QString &timeStr) {
    QString format = "yyyy-MM-dd HH:mm:ss";

    return QDateTime::fromString(timeStr, format);
}

Sync::Sync(QObject *parent)
    : NetworkWorker(parent)
    , mHasClient(false)
{
    QSettings setting;
    mHasClient            = setting.value(m_HasClientSetting).toBool();
    mSerialNumber         = setting.value(m_SerialNumberSetting).toString();
    mContractorInfo       = setting.value(m_ContractorSettings).toMap();
}

void Sync::setUID(cpuid_t accessUid)
{
    mSystemUuid = accessUid;
}

std::pair<std::string, bool> Sync::getSN(cpuid_t accessUid, bool notifyUser)
{
    auto netReply = sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getSn?uid=%0").arg(accessUid.c_str())), m_getSN);
    netReply->setProperty(m_notifyGetSN, notifyUser);

    // Return Serial number when serial number already exist.
    if (!mSerialNumber.isEmpty() && mHasClient)
        return getSN();

    QEventLoop loop;
    connect(this, &NUVE::Sync::snFinished, &loop, &QEventLoop::quit);

    loop.exec();

    TRACE << "Retrieve SN returned: " << QString::fromStdString(mSerialNumber.toStdString());

    return getSN();
}

std::pair<std::string, bool> Sync::getSN()
{
    return {mSerialNumber.toStdString(), mHasClient};
}

QVariantMap Sync::getContractorInfo()
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "ContractorInfo: The serial number is not recognized correctly...";
        return mContractorInfo;
    }

    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getContractorInfo?sn=%0").arg(mSerialNumber)), m_getContractorInfo);

    QEventLoop loop;
    connect(this, &NUVE::Sync::contractorInfoReady, &loop, &QEventLoop::quit);

    loop.exec();

    QSettings setting;
    setting.setValue(m_ContractorSettings, mContractorInfo);
    return mContractorInfo;
}

bool Sync::getSettings()
{
    QMutexLocker locker(&getSettingsMutex);
    bool returnval = false;

    if (getSettingsRequested) {
        TRACE_CHECK(true) << "skipping getSettings request due to previously active one";
        return false;
    }
    getSettingsRequested = true;
    locker.unlock();    // Let waiting threads exit

    if (mSerialNumber.isEmpty()) {
        qWarning()   << "Sn is not ready! can not get settings!";
    }
    else {
        sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getSettings?sn=%0").arg(mSerialNumber)), m_getSettings);

        QEventLoop loop;
        connect(this, &NUVE::Sync::settingsLoaded, &loop, &QEventLoop::quit);
        connect(this, &NUVE::Sync::settingsReady, &loop, [&loop] {
            loop.setProperty("success", true);
            loop.quit();
        });

        // Quit from loop and change success to 'true'
        connect(this, &NUVE::Sync::invalidSettingsReceived, &loop, [&loop] {
            loop.setProperty("success", true);
            loop.quit();
        });

        loop.exec(QEventLoop::ExcludeSocketNotifiers);

        if (getAutoModeSetings()) {
            returnval = loop.property("success").toBool();
        }
    }

    locker.relock();
    getSettingsRequested = false;
    return returnval;
}

bool Sync::getAutoModeSetings() {
    if (mSerialNumber.isEmpty()) {
        qWarning()   << "Sn is not ready! can not get auto mode settings!";
        return false;
    }

    sendGetRequest(m_domainUrl, QUrl(QString("/api/sync/autoMode?sn=%0").arg(mSerialNumber)), m_getAutoModeSettings);

    QEventLoop loop;
    connect(this, &NUVE::Sync::autoModeSettingsReady, &loop, [&loop](QVariantMap settings, bool isValid) {
        loop.setProperty("success", isValid);
        loop.quit();
    });

    loop.exec(QEventLoop::ExcludeSocketNotifiers);
    return loop.property("success").toBool();
}

void Sync::getMessages()
{
    if (mSerialNumber.isEmpty()) {
        qWarning()   << "Sn is not ready! can not get messages!";
        return;
    }

    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/messages?sn=%0").arg(mSerialNumber)), m_getMessages);

    QEventLoop loop;
    connect(this, &NUVE::Sync::messagesLoaded, &loop, &QEventLoop::quit);

    loop.exec();
}

void Sync::getWirings(cpuid_t accessUid)
{
    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getWirings?uid=%0").arg(accessUid.c_str())), m_getWirings);

    QEventLoop loop;
    connect(this, &NUVE::Sync::wiringReady, &loop, &QEventLoop::quit);

    loop.exec();
}

QByteArray Sync::preparePacket(QString className, QString method, QJsonArray params)
{
    QJsonObject requestData;
    requestData["request"] = QJsonObject{
        {"class", className},
        {"method", method},
        {"params", params}
    };

    requestData["user"] = QJsonObject{
        {"lang_id", 0},
        {"user_id", 0},
        {"type_id", 0},
        {"host_id", 0},
        {"region_id", 0},
        {"token", ""}
    };

    QJsonDocument jsonDocument(requestData);

    return jsonDocument.toJson();
}

void Sync::requestJob(QString type)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(type);

    QByteArray requestData = preparePacket("sync", m_requestJob, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_requestJob);
}

void Sync::pushSettingsToServer(const QVariantMap &settings)
{
    QJsonObject requestDataObj = QJsonObject::fromVariantMap(settings);
    requestDataObj["sn"] = mSerialNumber;

    QJsonDocument jsonDocument(requestDataObj);

    QByteArray requestData = jsonDocument.toJson();


    sendPostRequest(m_domainUrl, QUrl(QString("/api/sync/update")), requestData, m_setSettings);
}

void Sync::pushAutoSettingsToServer(const double& auto_temp_low, const double& auto_temp_high)
{
    QJsonObject requestDataObj;
    requestDataObj["auto_temp_low"] = auto_temp_low;
    requestDataObj["auto_temp_high"] = auto_temp_high;

    // Temporary, has no effect
    requestDataObj["mode"] = "auto";
    requestDataObj["is_active"] = true;

    QJsonDocument jsonDocument(requestDataObj);

    QByteArray requestData = jsonDocument.toJson();


    sendPostRequest(m_domainUrl, QUrl(QString("/api/sync/autoMode?sn=%0").arg(mSerialNumber)), requestData, m_setAutoModeSettings);
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

    QJsonDocument jsonDocument(requestDataObj);

    QByteArray requestData = jsonDocument.toJson();


    TRACE_CHECK(false) << requestData.toStdString().c_str();
    sendPostRequest(m_domainUrl, QUrl(QString("/api/sync/alerts")), requestData, m_setAlerts);
}

void Sync::ForgetDevice()
{
    mHasClient = false;
    mSerialNumber = QString();
    mContractorInfo = QVariantMap {};

    // Save the serial number in settings
    QSettings setting;
    setting.setValue(m_HasClientSetting, mHasClient);
    setting.setValue(m_SerialNumberSetting, mSerialNumber);
    setting.setValue(m_ContractorSettings, mContractorInfo);
}

void Sync::processNetworkReply(QNetworkReply* reply)
{
    QString errorString = reply->error() == QNetworkReply::NoError ? "" : reply->errorString();
    auto method = reply->property(m_methodProperty).toString();

    QByteArray dataRaw = reply->readAll();
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(dataRaw);
    const QJsonObject jsonDocObj = jsonDoc.object();
    if (errorString.isEmpty() && !jsonDocObj.contains("data") && method != m_getContractorLogo) {
        errorString = "server returned null response";
    }

    if (errorString.isEmpty()) {
        auto dataValue = jsonDocObj.value("data");

        switch (reply->operation()) {
        case QNetworkAccessManager::PostOperation: {
            TRACE_CHECK(false) << method << dataRaw << jsonDocObj << dataValue << dataValue.isObject() << jsonDoc.toJson().toStdString().c_str();

            if (method == m_setSettings) {
                // get the last update
                auto dateString = jsonDocObj.value("data").toObject().value("setting").toObject().value("last_update");
                TRACE << "cpp set last_update:" << dateString;
                QDateTime dateTimeObject = updateTimeStringToTime(dateString.toString());

                if (dateTimeObject.isValid()) {
                    // Use the dateTimeObject here with time information
                    TRACE << "Date with time cpp set last_update: " << dateTimeObject << dateTimeObject.toString() ;
                    mLastPushTime = dateTimeObject;
                } else {
                    TRACE << "Invalid date format! cpp set last_update:";
                }

                Q_EMIT pushSuccess();

            } else if (method == m_setAutoModeSettings) {

                TRACE << "Auto mode settings pushed to server: " << m_setAutoModeSettings;
                Q_EMIT autoModePush(true);
            }


        } break;
        case QNetworkAccessManager::GetOperation: {
            TRACE << dataValue.isObject() << reply->property(m_methodProperty).toString();
            TRACE_CHECK(method != m_getContractorLogo && method != m_getSettings) << dataRaw << jsonDocObj;

            if (method == m_getSN) {
                if (dataValue.isObject())
                {
                    auto dataObj = dataValue.toObject();
                    if (!dataObj.contains("serial_number")) {
                        errorString = "No serial number has returned by server";
                        break;
                    }

                    auto sn = dataObj.value("serial_number").toString();
                    mHasClient = dataObj.value("has_client").toBool();
                    TRACE << sn << mHasClient;

                    if (!mHasClient) {
                        TRACE << "will start initial setup!";
                    }

                    if (!mSerialNumber.isEmpty() && sn != mSerialNumber){
                        emit alert("The serial number does not match the last one.");

                        if (!mHasClient) {
                            // Update SN for get settings
                            mSerialNumber = sn;
                            // Force to update with new settings
                            mLastPushTime = QDateTime();
                            // Fetch with new serial number
                            emit serialNumberChanged();
                        }

                        TRACE << "The serial number does not match the last one." << mSerialNumber << sn;
                    } else if (sn.isEmpty()) {
                        emit alert("Oops...\nlooks like this device is not recognized by our servers,\nplease send it to the manufacturer and\n try to install another device.");
                    }

                    mSerialNumber = sn;

                    auto notifyUser = reply->property(m_notifyGetSN).toBool();
                    if (mSerialNumber.isEmpty() && notifyUser)
                        emit testModeStarted();

                    // Send sn ready signal to get software update.
                    if (!mSerialNumber.isEmpty())
                        Q_EMIT snReady();

                    // Save the serial number in settings
                    QSettings setting;
                    setting.setValue(m_HasClientSetting, mHasClient);
                    setting.setValue(m_SerialNumberSetting, mSerialNumber);

                    Q_EMIT snFinished();
                } else {
                    errorString = "No serial number has returned by server";
                }
            } else if (method == m_getContractorInfo) {
                if (dataValue.isObject())
                {
                    auto dataObj = dataValue.toObject();
                    auto brandValue = dataObj.value("brand");
                    auto phoneValue = dataObj.value("phone");
                    auto logoValue = dataObj.value("logo");

                    if (!brandValue.isString() || brandValue.toString().isEmpty()) {
                        errorString = "Wrong contractor info fetched from server";
                        break;
                    }

                    QVariantMap map;
                    map.insert("phone", phoneValue.toString(mContractorInfo.value("phone").toString()));
                    map.insert("brand", brandValue.toString(mContractorInfo.value("brand").toString()));
                    map.insert("url", dataObj.value("url").toString(mContractorInfo.value("url").toString()));
                    map.insert("tech", dataObj.value("schedule").toString(mContractorInfo.value("tech").toString()));
                    // logo is a bit more complicated than others,
                    // the value inside map should be either empty so it loads from brand name, or be a resource or local fs path
                    // so if it has the logo response we keep it empty until the actual value handled (if not empty, it should be downloaded to a local path)
                    // and if it has null or has not value, we will keep the previous value
                    map.insert("logo", logoValue.isString() ? "" : mContractorInfo.value("logo").toString());
                    mContractorInfo = map;

                    auto logo = logoValue.toString();
                    if (logo.isEmpty()){
                        Q_EMIT contractorInfoReady();
                    } else {
                        // what if gets error, should we return immadiately?
                        QNetworkRequest dlRequest(logo);
                        QNetworkReply *netReply = get(dlRequest);
                        netReply->setProperty(m_methodProperty, m_getContractorLogo);
                    }
                } else {
                    errorString = "Wrong contractor info fetched from server";
                }
            } else if (method == m_getContractorLogo) {
                QImage image;
                if (image.loadFromData(dataRaw)){
                    image.save("/home/root/customIcon.png");
                    mContractorInfo.insert("logo", "file:///home/root/customIcon.png");
                }
                Q_EMIT contractorInfoReady();
            } else if (method == m_getSettings) {
                TRACE_CHECK(false) << jsonDoc.toJson().toStdString().c_str();


                if (jsonDoc.isObject()) {
                    auto data = jsonDoc.object().value("data");
                    if (data.isObject()){
                        auto object = data.toObject();
                        if (object.value("sn").toString() == mSerialNumber){
                            auto dateString = object.value("setting").toObject().value("last_update");
                            TRACE << "cpp last_update:" << dateString;
                            QDateTime dateTimeObject = updateTimeStringToTime(dateString.toString());

                            if (dateTimeObject.isValid()) {
                                // Use the dateTimeObject here with time information
                                TRACE << "Date with time cpp set last_update: " << dateTimeObject << dateTimeObject.toString()
                                           << (mLastPushTime > dateTimeObject) << (mLastPushTime == dateTimeObject) << (mLastPushTime < dateTimeObject) ;
                            } else {
                                TRACE << "Invalid date format! cpp set last_update:";
                            }
                            if (!mLastPushTime.isNull() && (!dateTimeObject.isValid() || mLastPushTime >= dateTimeObject)) {
                                errorString = "Received settings has invalid date last_update: " + dateTimeObject.toString();
                                Q_EMIT invalidSettingsReceived();

                            } else {
                                mLastPushTime = dateTimeObject;
                                Q_EMIT settingsReady(object.toVariantMap());
                            }

                            // Transmit Non-Configuration Data to UI and Update Model on Server Response
                            emit appDataReady(object.toVariantMap());

                            checkFirmwareUpdate(object);

                            break;
                        } else {
                            errorString = "Received settings belong to another device: " + mSerialNumber + ", " + object.value("sn").toString();
                            break;
                        }
                    }
                }

                errorString = "Received settings corrupted: " + mSerialNumber ;

            } else if (method == m_getMessages) {
                TRACE << jsonDoc.toJson().toStdString().c_str();
                Q_EMIT messagesLoaded();
            } else if (method == m_getWirings) {
                TRACE ;
                Q_EMIT wiringReady();

            } else if (method == m_getAutoModeSettings) {
                TRACE_CHECK(true) << jsonDoc.toJson().toStdString().c_str();
                if (jsonDoc.isObject()) {
                    auto data = jsonDoc.object().value("data");
                    if (data.isObject()) {
                        auto object = data.toObject();
                        Q_EMIT autoModeSettingsReady(object.toVariantMap(), true);

                    } else {
                        errorString = "Received settings corrupted";
                        break;
                    }
                }
            }
        } break;

        default:

            break;
        }
    }

    if (!errorString.isEmpty()){
        if (method == m_getSN) {
            Q_EMIT snFinished();

            auto notifyUser = reply->property(m_notifyGetSN).toBool();
            if (notifyUser && reply->error() == QNetworkReply::ContentNotFoundError)
                emit testModeStarted();

            QString error = "Unable to fetch the device serial number, Please check your internet connection: ";
//            emit alert(error + errorString);
            qWarning() << error << errorString ;
        } else if (method == m_getContractorInfo) {
            Q_EMIT contractorInfoReady();
            QString error = "Unable to fetch the Contarctor Info, Please check your internet connection: ";
//            emit alert(error + errorString);
            qWarning() << error << errorString ;
        } else if (method == m_getContractorLogo) {
            Q_EMIT contractorInfoReady();
            QString error = "Unable to fetch the Contarctor logo, Please check your internet connection: ";
//            emit alert(error + errorString);
            qWarning() << error << errorString;
        } else if (method == m_getSettings) {
            QString error = "Unable to fetch the settings, Please check your internet connection: ";
            Q_EMIT settingsLoaded();
            qWarning() << error << errorString;
        } else if (method == m_getMessages) {
            QString error = "Unable to fetch the messages, Please check your internet connection: ";
            Q_EMIT messagesLoaded();
            qWarning() << error << errorString;
        } else if (method == m_setSettings) {
            QString error = "Unable to push the settings to server, Please check your internet connection: ";
            qWarning() << error << errorString;
            Q_EMIT pushFailed();

        } else if (method == m_getAutoModeSettings) {
            qWarning() << m_getAutoModeSettings << errorString;
            Q_EMIT autoModeSettingsReady(QVariantMap(), false);

        } else if (method == m_setAutoModeSettings) {
            QString error = "Unable to push the auto mode settings to server, Please check your internet connection: ";
            qWarning() << error << errorString;
            Q_EMIT autoModePush(false);

        } else {
            QString error = "unknown method in sync processNetworkReply ";
            qWarning() << method << error << errorString ;
        }
    }    
}

void Sync::checkFirmwareUpdate(QJsonObject settings)
{
    QString fwVersion;

    if (settings.contains(m_firmwareUpdateKey) &&
        settings.value(m_firmwareUpdateKey).isObject()) {
        auto fwUpdateObj = settings.value(m_firmwareUpdateKey).toObject();
        auto fwUpdateVersion = fwUpdateObj.value(m_firmwareImageKey).toString("");

        // if force-update is set to true, then firmware-image instructs device to update to that version
        if (fwUpdateObj.value(m_firmwareForceUpdateKey).toBool()) {
            fwVersion = fwUpdateVersion;
        }
    }

    emit updateFirmwareFromServer(fwVersion);
}

QNetworkReply* Sync::sendGetRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QString &method)
{
    // Prepare request
    QNetworkRequest netRequest(mainUrl.resolved(relativeUrl));
    netRequest.setRawHeader("accept", "application/json");
    netRequest.setTransferTimeout(4000);

    if (method != m_getSN) {
        auto data = mSystemUuid + mSerialNumber.toStdString();

        // Get error: QNetworkReply::ProtocolFailure "Server is unable to maintain the header compression context for the connection"
        netRequest.setRawHeader("Authorization", "Bearer " + QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
    }


    // Post a request
    QNetworkReply *netReply = get(netRequest);
    netReply->setProperty(m_methodProperty, method);
    return netReply;
}

void Sync::sendPostRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QByteArray &postData, const QString &method)
{
    // Prepare request
    QNetworkRequest netRequest(mainUrl.resolved(relativeUrl));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    netRequest.setRawHeader("accept", "application/json");
    netRequest.setTransferTimeout(8000);

    // set authentication
    {
        auto data = mSystemUuid + mSerialNumber.toStdString();

        // Get error: QNetworkReply::ProtocolFailure "Server is unable to maintain the header compression context for the connection"
        netRequest.setRawHeader("Authorization", "Bearer " + QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
    }

    // Post a request
    QNetworkReply *netReply = post(netRequest, postData);
    netReply->setProperty(m_methodProperty, method);
}
}


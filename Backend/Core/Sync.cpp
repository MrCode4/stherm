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
const QString m_getWirings = QString("getWirings");
const QString m_SerialNumberSetting = QString("NUVE/SerialNumber");
const QString m_HasClientSetting = QString("NUVE/SerialNumberClient");
const QString m_requestJob      = QString("requestJob");

Sync::Sync(QObject *parent) : NetworkWorker(parent),
    mHasClient(false)
{

    QSettings setting;
    mHasClient            = setting.value(m_HasClientSetting).toBool();
    mSerialNumber         = setting.value(m_SerialNumberSetting).toString();


    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &Sync::processNetworkReply);
}

void Sync::setUID(cpuid_t accessUid)
{
    mSystemUuid = accessUid;
}

std::pair<std::string, bool> Sync::getSN(cpuid_t accessUid)
{
    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getSn?uid=%0").arg(accessUid.c_str())), m_getSN);

    // Return Serial number when serial number already exist.
    if (!mSerialNumber.isEmpty() && mHasClient)
        return getSN();

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::Sync::snReady, &loop, &QEventLoop::quit);

    timer.start(100000); // 100 seconds TODO
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
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::Sync::contractorInfoReady, &loop, &QEventLoop::quit);

    timer.start(100000); // 100 seconds TODO
    loop.exec();

    return mContractorInfo;
}

void Sync::getSettings()
{
    if (mSerialNumber.isEmpty()) {
        qWarning()   << "Sn is not ready! can not get settings!";
        return;
    }

    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getSettings?sn=%0").arg(mSerialNumber)), m_getSettings);

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::Sync::settingsLoaded, &loop, &QEventLoop::quit);

    timer.start(100000); // 100 seconds TODO
    loop.exec();

}

void Sync::getWirings(cpuid_t accessUid)
{
    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getWirings?uid=%0").arg(accessUid.c_str())), m_getWirings);

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::Sync::wiringReady, &loop, &QEventLoop::quit);

    timer.start(100000); // 100 seconds TODO
    loop.exec();

}

void Sync::requestJob(QString type)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(type);

    QByteArray requestData = preparePacket("sync", m_requestJob, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_requestJob);
}

void Sync::processNetworkReply(QNetworkReply *netReply)
{
    NetworkWorker::processNetworkReply(netReply);

    QString errorString = netReply->error() == QNetworkReply::NoError ? "" : netReply->errorString();
    auto method = netReply->property(m_methodProperty).toString();

    QByteArray dataRaw = netReply->readAll();
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(dataRaw);
    const QJsonObject jsonDocObj = jsonDoc.object();
    if (errorString.isEmpty() && !jsonDocObj.contains("data")) {
        errorString = "server returned null response";
    }

    if (errorString.isEmpty()) {
        auto dataValue = jsonDocObj.value("data");

        switch (netReply->operation()) {
        case QNetworkAccessManager::PostOperation: {
            TRACE << dataRaw << jsonDocObj << dataValue << dataValue.isObject();

        } break;
        case QNetworkAccessManager::GetOperation: {
            TRACE << dataValue.isObject() << netReply->property(m_methodProperty).toString();
            TRACE_CHECK(method != m_getContractorLogo) << dataRaw << jsonDocObj;

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
                        TRACE << "The serial number does not match the last one." << mSerialNumber << sn;
                    } else if (sn.isEmpty()) {
                        emit alert("Oops...\nlooks like this device is not recognized by our servers,\nplease send it to the manufacturer and\n try to install another device.");
                    }

                    mSerialNumber = sn;

                    // Save the serial number in settings
                    QSettings setting;
                    setting.setValue(m_HasClientSetting, mHasClient);
                    setting.setValue(m_SerialNumberSetting, mSerialNumber);

                    Q_EMIT snReady();
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
                        QNetworkReply *netReply = mNetManager->get(dlRequest);
                        netReply->setProperty(m_methodProperty, m_getContractorLogo);
                    }
                } else {
                    errorString = "Wrong contractor info fetched from server";
                }
            }  else if (method == m_getContractorLogo) {
                QImage image;
                if (image.loadFromData(dataRaw)){
                    image.save("/home/root/customIcon.png");
                    mContractorInfo.insert("logo", "file:///home/root/customIcon.png");
                }
                Q_EMIT contractorInfoReady();
            } else if (method == m_getSettings) {
                TRACE;
                Q_EMIT settingsLoaded();
            } else if (method == m_getWirings) {
                TRACE ;
                Q_EMIT wiringReady();
            }
        } break;

        default:

            break;
        }
    }

    if (!errorString.isEmpty()){
        if (method == m_getSN) {
            Q_EMIT snReady();
            QString error = "Unable to fetch the device serial number, Please check your internet connection: ";
            emit alert(error + errorString);
            qWarning() << error << errorString ;
        } else if (method == m_getContractorInfo) {
            Q_EMIT contractorInfoReady();
            QString error = "Unable to fetch the Contarctor Info, Please check your internet connection: ";
            emit alert(error + errorString);
            qWarning() << error << errorString ;
        } else if (method == m_getContractorLogo) {
            Q_EMIT contractorInfoReady();
            QString error = "Unable to fetch the Contarctor logo, Please check your internet connection: ";
            emit alert(error + errorString);
            qWarning() << error << errorString ;
        } else {
            QString error = "unknown method in sync processNetworkReply ";
            qWarning() << method << error << errorString ;
        }
    }

    netReply->deleteLater();
}

void Sync::sendGetRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QString &method)
{
    // Prepare request
    QNetworkRequest netRequest(mainUrl.resolved(relativeUrl));
    netRequest.setRawHeader("accept", "application/json");

    if (method != m_getSN) {
        auto data = mSystemUuid + mSerialNumber.toStdString();

        // Get error: QNetworkReply::ProtocolFailure "Server is unable to maintain the header compression context for the connection"
        netRequest.setRawHeader("Authorization", "Bearer " + QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
    }


    // Post a request
    QNetworkReply *netReply = mNetManager->get(netRequest);
    netReply->setProperty(m_methodProperty, method);
    //    netReply->ignoreSslErrors();
}
}

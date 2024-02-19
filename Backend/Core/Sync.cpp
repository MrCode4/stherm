#include "Sync.h"
#include "LogHelper.h"

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
const QString m_getSettings = QString("getSettings");
const QString m_getWirings = QString("getWirings");
const QString m_SerialNumberSetting = QString("NUVE/SerialNumber");
const QString m_HasClientSetting = QString("NUVE/SerialNumberClient");
const QString m_requestJob      = QString("requestJob");

Sync::Sync(QObject *parent) : NetworkWorker(parent),
    mIsGetSNReceived(false), mHasClient(false)
{

    QSettings setting;
    mHasClient            = setting.value(m_HasClientSetting).toBool();
    mSerialNumber         = setting.value(m_SerialNumberSetting).toString();


    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &Sync::processNetworkReply);
}


std::pair<std::string, bool> Sync::getSN(cpuid_t accessUid)
{
    // allows maximum one time to fetch the sn from server
    if (mIsGetSNReceived) {
        return getSN();
    }

    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getSn?uid=%0").arg(accessUid.c_str())), m_getSN);

    // Return Serial number when serial number already exist.
    if (!mSerialNumber.isEmpty())
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

void Sync::getContractorInfo()
{
    if (mSerialNumber.isEmpty()) {
        qWarning() << "ContractorInfo: The serial number is not recognized correctly...";
        return;
    }

    sendGetRequest(m_domainUrl, QUrl(QString("api/sync/getContractorInfo?sn=%0").arg(mSerialNumber)), m_getContractorInfo);

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::Sync::contractorInfoReady, &loop, &QEventLoop::quit);

    timer.start(100000); // 100 seconds TODO
    loop.exec();
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

    if (netReply->error() != QNetworkReply::NoError){
        if (netReply->property(m_methodProperty).toString() == m_getSN) {
            //            emit alert("Unable to fetch the device serial number, Please check your internet connection: " + netReply->errorString());
        }

        netReply->deleteLater();
        return;
    }

    QByteArray data = netReply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonObject obj = doc.object();
    auto dataObj = obj.value("data");

    switch (netReply->operation()) {
    case QNetworkAccessManager::PostOperation: {
        TRACE << data << obj << dataObj << dataObj.isObject();

    } break;
    case QNetworkAccessManager::GetOperation: {
        TRACE << data << obj << dataObj << dataObj.isObject() << netReply->property(m_methodProperty).toString();

        if (netReply->property(m_methodProperty).toString() == m_getSN) {
            if (dataObj.isObject())
            {
                auto sn = dataObj.toObject().value("serial_number").toString();
                mHasClient = dataObj.toObject().value("has_client").toBool();
                TRACE << sn << mHasClient;

                if (!mHasClient) {
                    TRACE << "Should start initial setup!";
                }

                if (!mSerialNumber.isEmpty() && sn != mSerialNumber){
                //                    emit alert("The serial number does not match the last one.");
                }

                if (sn.isEmpty()) {
                    //                    emit alert("Oops...\nlooks like this device is not recognized by our servers,\nplease send it to the manufacturer and\n try to install another device.");
                    //                    mSerialNumber = ""; // TODO should we clear serial number when saved before but returns empty now?
                }

                mSerialNumber = sn;

                // Save the serial number in settings
                QSettings setting;
                setting.setValue(m_HasClientSetting, mHasClient);
                setting.setValue(m_SerialNumberSetting, mSerialNumber);

                Q_EMIT snReady();

                // prevents fetching again from server
                mIsGetSNReceived = true;
            }
        } else if (netReply->property(m_methodProperty).toString() == m_getContractorInfo) {
            TRACE;

            // TODO: complete contractor information.
            auto resultObj = obj.value("result").toObject().value("result").toObject();
            TRACE << resultObj;
            QVariantMap map;
            map.insert("phone", resultObj.value("phone").toString());
            map.insert("name", resultObj.value("name").toString());
            map.insert("url", resultObj.value("url").toString());
            map.insert("techLink", resultObj.value("tech_link").toString());


            Q_EMIT contractorInfoReady();
        } else if (netReply->property(m_methodProperty).toString() == m_getSettings) {
            TRACE;

            Q_EMIT settingsLoaded();
        } else if (netReply->property(m_methodProperty).toString() == m_getWirings) {
            TRACE ;
            Q_EMIT wiringReady();
        }
    } break;

    default:

        break;
    }
    netReply->deleteLater();
}

void Sync::sendGetRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QString &method)
{
    // Prepare request
    QNetworkRequest netRequest(mainUrl.resolved(relativeUrl));
    netRequest.setRawHeader("accept", "application/json");

    // Post a request
    QNetworkReply *netReply = mNetManager->get(netRequest);
    netReply->setProperty(m_methodProperty, method);
    //    netReply->ignoreSslErrors();
}
}

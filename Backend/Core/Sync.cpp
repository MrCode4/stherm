#include "Sync.h"
#include "LogHelper.h"

#include <QUrl>

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
namespace NUVE {
const QUrl m_domainUrl        = QUrl("http://test.hvac.z-soft.am"); // base domain
const QUrl m_engineUrl        = QUrl("/engine/index.php");          // engine
const QUrl m_updateUrl        = QUrl("/update/");                   // update
const QString m_getSN         = QString("getSN");
const QString m_SerialNumberSetting        = QString("Stherm/SerialNumber");

Sync::Sync(QObject *parent) : NetworkWorker(parent),
    mIsGetSNReceived(false)
{

    QSettings setting;
    mSerialNumber            = setting.value(m_SerialNumberSetting).toString();


    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &Sync::processNetworkReply);
}

void Sync::changeContractorInfo(QString serialNumber)
{
    QJsonArray paramsArray;
    paramsArray.append(serialNumber);

    QByteArray requestData = preparePacket("sync", "getContractorInfo", paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData);
}

std::string Sync::getSN(cpuid_t accessUid)
{
    if (mIsGetSNReceived) {
        return mSerialNumber.toStdString();
    }

    QJsonArray paramsArray;
    paramsArray.append(QString::fromStdString(accessUid));

    // TODO parameter retrieval from cloud, can we utilise a value/isSet tuple and push the processing to a background function?  Or are we happy with a firing off a whole bunch of requests and waiting for them to complete?
    QByteArray requestData = preparePacket("sync", m_getSN, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_getSN);

    // Return Serial number when serial number already exist.
    if (!mSerialNumber.isEmpty())
        return mSerialNumber.toStdString();

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::Sync::snReady, &loop, &QEventLoop::quit);

    timer.start(100000); // 100 seconds TODO
    loop.exec();

    TRACE << "Retrieve SN returned: " << QString::fromStdString(mSerialNumber.toStdString());

    return mSerialNumber.toStdString();
}

std::string Sync::getSN()
{
    return mSerialNumber.toStdString();
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

    switch (netReply->operation()) {
    case QNetworkAccessManager::PostOperation: {
        if (netReply->property(m_methodProperty).toString() == m_getSN) {
            QJsonArray resultArray = obj.value("result").toObject().value("result").toArray();
            qDebug() << Q_FUNC_INFO << __LINE__ << resultArray;

            if (resultArray.count() > 0) {
                auto sn = resultArray.first().toString();

                if (!mSerialNumber.isEmpty() && sn != mSerialNumber);
                //                    emit alert("The serial number does not match the last one.");

                if (sn.isEmpty()) {
                    //                    emit alert("Oops...\nlooks like this device is not recognized by our servers,\nplease send it to the manufacturer and\n try to install another device.");

                } else {
                    mSerialNumber = sn;
                }

                // Save the serial number in settings
                QSettings setting;
                setting.setValue(m_SerialNumberSetting, mSerialNumber);

                Q_EMIT snReady();

                mIsGetSNReceived = true;
            }
        }

        if (netReply->url().toString().endsWith(m_engineUrl.toString())) {
            qDebug() << Q_FUNC_INFO << __LINE__ << obj;
            QJsonArray resultArray = obj.value("result").toObject().value("result").toArray();
        }

    } break;
    case QNetworkAccessManager::GetOperation: {

    } break;

    default:

        break;
    }
    netReply->deleteLater();
}
}

#include "System.h"

#include <QProcess>
#include <QDebug>
#include <QUrl>

/* ************************************************************************************************
 * Network information
 * ************************************************************************************************/
const QUrl m_domainUrl        = QUrl("http://test.hvac.z-soft.am"); // base domain
const QUrl m_engineUrl        = QUrl("/engine/index.php");          // engine
const QUrl m_updateUrl        = QUrl("/update/");                   // update

const QString m_getSN           = QString("getSN");
const QString m_getSystemUpdate = QString("getSystemUpdate");
const QString m_requestJob      = QString("requestJob");
const QString m_partialUpdate   = QString("partialUpdate");

NUVE::System::System(QObject *parent)
{
    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &System::processNetworkReply);
}

std::string NUVE::System::getSN(cpuid_t accessUid)
{
    QJsonArray paramsArray;
    paramsArray.append(QString::fromStdString(accessUid));

    // TODO parameter retrieval from cloud, can we utilise a value/isSet tuple and push the processing to a background function?  Or are we happy with a firing off a whole bunch of requests and waiting for them to complete?
    QByteArray requestData = preparePacket("sync", m_getSN, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_getSN);

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &NUVE::System::snReady, &loop, &QEventLoop::quit);
    // TODO this timeout is enough for a post request?
    // TODO the timeout needs to be defined in a paramter somewhere
    timer.start(3000);
    loop.exec();

    qDebug() << Q_FUNC_INFO << "Retrieve SN returned: " << QString::fromStdString(mSerialNumber.toStdString());

    return mSerialNumber.toStdString();
}

void NUVE::System::getUpdate(QString softwareVersion)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(softwareVersion);

    QByteArray requestData = preparePacket("sync", m_getSystemUpdate, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_getSystemUpdate);
}

void NUVE::System::requestJob(QString type)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(type);

    QByteArray requestData = preparePacket("sync", m_requestJob, paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData, m_requestJob);
}

void NUVE::System::partialUpdate(const QJsonObject &jsonObj) {
    // Extracting values from JSON
    QString hv = jsonObj["hv"].toString();
    QString require = jsonObj["require"].toString();
    QString sv = jsonObj["sv"].toString();
    QString type = jsonObj["type"].toString();
    QString filename = jsonObj["list"].toArray()[0].toObject()["filename"].toString();
    QString url = jsonObj["list"].toArray()[0].toObject()["url"].toString();

    // Construct web file URL
    QString web_file = m_domainUrl.toString() + m_updateUrl.toString() +
                       hv + require + sv + type + "/" + filename;

    // Fetch the file from web location
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(web_file)));
    reply->setProperty(m_methodProperty, m_partialUpdate);

    connect(reply, &QNetworkReply::downloadProgress, this, [=] (qint64 bytesReceived, qint64 bytesTotal) {
        qDebug() << Q_FUNC_INFO << __LINE__ << bytesReceived << bytesTotal;
    });

}
void NUVE::System::processNetworkReply(QNetworkReply *netReply)
{
    NetworkWorker::processNetworkReply(netReply);

    if (netReply->error() != QNetworkReply::NoError)
        return;

    QByteArray data = netReply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonObject obj = doc.object();

    switch (netReply->operation()) {
    case QNetworkAccessManager::PostOperation: {

        if (netReply->property(m_methodProperty).toString() == m_getSN) {
            QJsonArray resultArray = obj.value("result").toObject().value("result").toArray();
            qDebug() << Q_FUNC_INFO << __LINE__ << resultArray;


            if (resultArray.count() > 0) {
                mSerialNumber = resultArray.first().toString();
                Q_EMIT snReady();
            }
        } else if (netReply->property(m_methodProperty).toString() == m_getSystemUpdate) {
            auto resultObj = obj.value("result").toObject().value("result").toObject();
            auto isRequire = (resultObj.value("require").toString() == "r");
            auto updateType = resultObj.value("type").toString();

            if (updateType == "f") {
                qDebug() << Q_FUNC_INFO << __LINE__ << "The system requires a full update.";

            } else {
                qDebug() << Q_FUNC_INFO << __LINE__ << "The system requires a partial update.";

                partialUpdate(resultObj);

            }
        }

    } break;
    case QNetworkAccessManager::GetOperation: {

        // Partial update (download process) finished.
        if (netReply->property(m_methodProperty).toString() == m_partialUpdate) {

        }
    } break;

    default:

        break;
    }
}

void NUVE::System::rebootDevice()
{
        QProcess process;
        QString command = "sudo reboot";

        process.start(command);
        process.waitForFinished();

        int exitCode = process.exitCode();
        QByteArray result = process.readAllStandardOutput();
        QByteArray error = process.readAllStandardError();

        qDebug() << "Exit Code:" << exitCode;
        qDebug() << "Standard Output:" << result;
        qDebug() << "Standard Error:" << error;
}

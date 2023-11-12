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


System::System(QObject *parent)
{
    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &System::processNetworkReply);
}

void System::getQR(QString accessUid)
{
    QJsonArray paramsArray;
    paramsArray.append(accessUid);

    QByteArray requestData = preparePacket("sync", "getSN", paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData);
}

void System::getUpdate(QString softwareVersion)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(softwareVersion);

    QByteArray requestData = preparePacket("sync", "getSystemUpdate", paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData);
}

void System::requestJob(QString type)
{
    QJsonArray paramsArray;
    paramsArray.append(mSerialNumber);
    paramsArray.append(type);

    QByteArray requestData = preparePacket("sync", "requestJob", paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData);
}

void System::processNetworkReply (QNetworkReply * netReply) {

    NetworkWorker::processNetworkReply(netReply);

    if (netReply->error() != QNetworkReply::NoError)
        return;

    QByteArray data = netReply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonObject obj = doc.object();

    switch (netReply->operation()) {
    case QNetworkAccessManager::PostOperation: {

        if (netReply->url().toString().endsWith(m_engineUrl.toString())) {
            qDebug() << Q_FUNC_INFO << __LINE__ << obj;
            QJsonArray resultArray = obj.value("result").toObject().value("result").toArray();

            if (resultArray.count() > 0)
                mSerialNumber = resultArray.first().toString();
        }

    } break;
    case QNetworkAccessManager::GetOperation: {

    } break;

    default:

        break;
    }
}

void System::rebootDevice()
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

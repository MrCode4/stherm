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

std::string System::getSN(cpuid_t accessUid)
{
    QJsonArray paramsArray;
    paramsArray.append(QString::fromStdString(accessUid));

// TODO parameter retrieval from cloud, can we utilise a value/isSet tuple and push the processing to a background function?  Or are we happy with a firing off a whole bunch of requests and waiting for them to complete?
    QByteArray requestData = preparePacket("sync", "getSN", paramsArray);
    sendPostRequest(m_domainUrl, m_engineUrl, requestData);

    QEventLoop loop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(this, &System::snReady, &loop, &QEventLoop::quit);
// TODO this timeout is enough for a post request?
// TODO the timeout needs to be defined in a paramter somewhere
    timer.start(1000);
    loop.exec();

    qDebug() << Q_FUNC_INFO << "Retrieve SN returned: " << QString::fromStdString(mSerialNumber.toStdString());

    return mSerialNumber.toStdString();
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

            if (resultArray.count() > 0) {
                mSerialNumber = resultArray.first().toString();
                Q_EMIT snReady();
            }
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

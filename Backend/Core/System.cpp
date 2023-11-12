#include "System.h"

#include <QProcess>
#include <QDebug>
#include <QUrl>

/* ************************************************************************************************
 * Authentication information
 * ************************************************************************************************/

const QUrl m_domainUrl        = QUrl("http://test.hvac.z-soft.am"); // base domain
const QUrl m_engineUrl        = QUrl("/engine/index.php");          // engine
const QUrl m_updateUrl         = QUrl("/update/");                   // update


System::System(QObject *parent) :
    QObject(parent)
{
    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &AuthenticationCPP::processNetworkReply);
}

void System::processNetworkReply (QNetworkReply * netReply) {

    // Handle Errors
    if (netReply->error() != QNetworkReply::NoError) {
        qDebug() << Q_FUNC_INFO <<__LINE__<< netReply->error()<<netReply->errorString();
        const QJsonObject errObj = QJsonDocument::fromJson(netReply->readAll()).object();
        QStringList errMsg = errObj.value("non_field_errors").toVariant().toStringList();

        // Remove url from error.
        QString error = netReply->errorString().remove(netReply->request().url().toString());
//        emit logInError(errMsg.isEmpty() ? error : errMsg.join("\n"));

        return;
    }

    QByteArray data = netReply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    const QJsonObject obj = doc.object();

    switch (netReply->operation()) {
    case QNetworkAccessManager::PostOperation: {

    } break;
    case QNetworkAccessManager::GetOperation: {

    } break;

    default:

        break;
    }
}

void System::sendPostRequest(const QUrl& relativeUrl, const QByteArray& postData)
{
    // Prepare request
    QNetworkRequest netRequest(m_domainUrl.resolved(relativeUrl));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Post a request
    QNetworkReply *netReply = mNetManager->post(netRequest, postData);
    netReply->ignoreSslErrors();
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

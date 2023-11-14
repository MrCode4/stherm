#include "NetworkWorker.h"

NetworkWorker::NetworkWorker(QObject *parent)
{
    mNetManager = new QNetworkAccessManager();

    connect(mNetManager, &QNetworkAccessManager::finished, this,  &NetworkWorker::processNetworkReply);

}

QByteArray NetworkWorker::preparePacket(QString className, QString method, QJsonArray params)
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

void NetworkWorker::sendPostRequest(const QUrl &mainUrl, const QUrl &relativeUrl, const QByteArray &postData)
{
    // Prepare request
    QNetworkRequest netRequest(mainUrl.resolved(relativeUrl));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Post a request
    QNetworkReply *netReply = mNetManager->post(netRequest, postData);
    //    connect(netReply, &QNetworkReply::finished, )
    //    netReply->ignoreSslErrors();
}

void NetworkWorker::processNetworkReply(QNetworkReply *netReply)
{
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
}

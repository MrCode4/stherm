#include "RestApiExecutor.h"

#include "LogHelper.h"

#include <QNetworkAccessManager>

RestApiExecutor::RestApiExecutor(QObject *parent)
    : HttpExecutor(parent)
{
}

void RestApiExecutor::setApiAuth(QNetworkRequest& request) {}

QString RestApiExecutor::prepareHashKey(int operation, const QString& endpoint)
{
    return QString("%1:%2").arg(operation).arg(endpoint);
}

QNetworkRequest RestApiExecutor::prepareApiRequest(const QString &endpoint, bool setAuth)
{
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("accept", "application/json");
    request.setTransferTimeout(8000);
    if (setAuth) setApiAuth(request);
    return request;
}

QNetworkReply* RestApiExecutor::callGetApi(const QString &endpoint, ResponseCallback callback, bool setAuth)
{
    auto key = prepareHashKey(QNetworkAccessManager::Operation::GetOperation, endpoint);
    if (mCallbacks.contains(key)) {
        return nullptr;
    }
    else {
        mCallbacks.insert(key, callback);
        auto reply = get(prepareApiRequest(endpoint, setAuth));
        reply->setProperty("endpoint", endpoint);
        reply->setProperty("isJson", true);
        return reply;
    }
}

QNetworkReply* RestApiExecutor::callPostApi(const QString &endpoint, const QByteArray &postData, ResponseCallback callback, bool setAuth)
{
    auto key = prepareHashKey(QNetworkAccessManager::Operation::PostOperation, endpoint);
    if (mCallbacks.contains(key)) {
        return nullptr;
    }
    else {
        mCallbacks.insert(key, callback);
        auto reply = post(prepareApiRequest(endpoint, setAuth), postData);
        reply->setProperty("endpoint", endpoint);
        reply->setProperty("isJson", true);
        return reply;
    }
}

QNetworkReply* RestApiExecutor::downloadFile(const QString &url, ResponseCallback callback, bool jsonFile, bool setAuth)
{
    auto key = prepareHashKey(QNetworkAccessManager::Operation::GetOperation, url);
    if (mCallbacks.contains(key)) {
        return nullptr;
    }
    else {
        QNetworkRequest request(url);
        if (setAuth) setApiAuth(request);
        QNetworkReply *reply = get(request);
        reply->setProperty("isJson", jsonFile);
        reply->setProperty("endpoint", url);
        mCallbacks.insert(key, callback);
        return reply;
    }
}

QJsonObject RestApiExecutor::prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const
{
    return QJsonDocument::fromJson(rawData).object();
}

void RestApiExecutor::processNetworkReply(QNetworkReply *reply)
{
    auto endpoint = reply->property("endpoint").toString();
    auto key = prepareHashKey(reply->operation(), endpoint);
    if (!mCallbacks.contains(key)) {
        TRACE << "Callback not found for endpoint " << endpoint;
        return;
    }

    QByteArray rawData;
    QJsonObject data;

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "API ERROR (" << endpoint << ") # code: "<< reply->error() << ", message: " << reply->errorString();
    }
    else {
        rawData = reply->readAll();

        if (reply->property("isJson").isValid() && reply->property("isJson").value<bool>()) {

            data = prepareJsonResponse(endpoint, rawData);
            if (data.isEmpty()) {
                TRACE << "API RESPONSE (" << endpoint << ") is Empty or invalid:" << rawData;

            } else if (!reply->property("noContentLog").isValid() || !reply->property("noContentLog").value<bool>()) {
                QJsonDocument doc(data);
                QString strJson(doc.toJson(QJsonDocument::Indented));
                TRACE << "API RESPONSE (" << endpoint << ") : " << strJson.toStdString().c_str();
            }
        }
    }

    auto callback = mCallbacks.take(key);
    if (callback) {
        callback(reply, rawData, data);
    }
}

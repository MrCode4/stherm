#include "RestApiExecutor.h"

#include "LogHelper.h"

RestApiExecutor::RestApiExecutor(QObject *parent)
    : HttpExecutor(parent)
{
}

void RestApiExecutor::setApiAuth(QNetworkRequest& request) {}

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
    if (mCallbacks.contains(endpoint)) {
        return nullptr;
    }
    else {
        mCallbacks.insert(endpoint, callback);
        auto reply = get(prepareApiRequest(endpoint, setAuth));
        reply->setProperty("endpoint", endpoint);
        return reply;
    }
}

QNetworkReply* RestApiExecutor::callPostApi(const QString &endpoint, const QByteArray &postData, ResponseCallback callback, bool setAuth)
{
    if (mCallbacks.contains(endpoint)) {
        return nullptr;
    }
    else {
        mCallbacks.insert(endpoint, callback);
        auto reply = post(prepareApiRequest(endpoint, setAuth), postData);
        reply->setProperty("endpoint", endpoint);
        return reply;
    }
}

QNetworkReply* RestApiExecutor::downloadFile(const QString &url, ResponseCallback callback, bool setAuth)
{
    if (mCallbacks.contains(url)) {
        return nullptr;
    }
    else {
        QNetworkRequest request(url);
        if (setAuth) setApiAuth(request);
        QNetworkReply *reply = get(request);
        reply->setProperty("IsFileData", true);
        reply->setProperty("endpoint", url);
        mCallbacks.insert(url, callback);
        return reply;
    }
}

void RestApiExecutor::processNetworkReply(QNetworkReply *reply)
{
    auto endpoint = reply->property("endpoint").toString();
    if (!mCallbacks.contains(endpoint)) {
        TRACE << "Callback not found for endpoint " << endpoint;
        return;
    }

    QByteArray rawData;
    QJsonObject data;

    if (reply->error() != QNetworkReply::NoError) {
        TRACE << "API ERROR (" << endpoint << ") # code: "<< reply->error() << ", message: " << reply->errorString();
        processApiError(endpoint, reply);
    }
    else {
        rawData = reply->readAll();

        if (reply->property("IsFileData").isValid() == false) {
            const QJsonObject jsonDocObj = QJsonDocument::fromJson(rawData).object();

            if (reply->property("noContentLog").isValid() == false) {
                TRACE << "API RESPONSE (" << endpoint << ") : " << rawData;
            }

            if (jsonDocObj.contains("data")) {
                data = jsonDocObj.value("data").toObject();
            }
            else {
                TRACE << "API ERROR (" << endpoint << ") : " << " Reponse contains no data object";
            }
        }

        processApiSuccess(endpoint, rawData, data);
    }

    auto callback = mCallbacks.take(endpoint);
    if (callback) {
        callback(reply, rawData, data);
    }
}

void RestApiExecutor::processApiSuccess(const QString& endpoint, const QByteArray& rawData, QJsonObject& data)
{

}

void RestApiExecutor::processApiError(const QString& endpoint, QNetworkReply* reply)
{

}


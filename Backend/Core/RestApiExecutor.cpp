#include "RestApiExecutor.h"
#include "LogHelper.h"

#include <QNetworkAccessManager>
#include <QUrl>

RestApiExecutor::RestApiExecutor(QObject *parent)
    : HttpExecutor(parent)
{
}

void RestApiExecutor::setApiAuth(QNetworkRequest& request) {}

QString RestApiExecutor::prepareHashKey(int operation, const QString& endpoint)
{
    return QString("%1:%2").arg(operation).arg(endpoint);
}

QNetworkRequest RestApiExecutor::prepareApiRequest(const QString &endpoint, bool setAuth, QString contentTypeHeader)
{
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentTypeHeader);
    request.setRawHeader("accept", "application/json");
    request.setTransferTimeout(20000);
    if (setAuth) setApiAuth(request);
    return request;
}

QNetworkReply* RestApiExecutor::callGetApi(const QString &endpoint, ResponseCallback callback, bool setAuth, QString contentTypeHeader)
{
    auto key = prepareHashKey(QNetworkAccessManager::Operation::GetOperation, endpoint);
    if (mCallbacks.contains(key)) {
        return nullptr;
    }
    else {
        mCallbacks.insert(key, callback);
        auto reply = get(prepareApiRequest(endpoint, setAuth, contentTypeHeader));
        if (reply) {
            reply->setProperty("endpoint", endpoint);
            reply->setProperty("isJson", true);
        }
        else {
            mCallbacks.remove(key);
        }
        return reply;
    }
}

QNetworkReply* RestApiExecutor::callPostApi(const QString &endpoint, const QByteArray &postData, ResponseCallback callback, bool setAuth, QString contentTypeHeader)
{
    auto key = prepareHashKey(QNetworkAccessManager::Operation::PostOperation, endpoint);
    if (mCallbacks.contains(key)) {
        return nullptr;
    }
    else {
        mCallbacks.insert(key, callback);
        auto reply = post(prepareApiRequest(endpoint, setAuth, contentTypeHeader), postData);
        if (reply) {
            reply->setProperty("endpoint", endpoint);
            reply->setProperty("isJson", true);
        }
        else {
            mCallbacks.remove(key);
        }
        return reply;
    }
}

QNetworkReply* RestApiExecutor::callPutApi(const QString &endpoint, const QByteArray &postData, ResponseCallback callback, bool setAuth, QString contentTypeHeader)
{
    auto key = prepareHashKey(QNetworkAccessManager::Operation::PutOperation, endpoint);
    if (mCallbacks.contains(key)) {
        return nullptr;
    }
    else {
        mCallbacks.insert(key, callback);
        auto reply = put(prepareApiRequest(endpoint, setAuth, contentTypeHeader), postData);
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
        mCallbacks.insert(key, callback);
        QNetworkRequest request(url);
        if (setAuth) setApiAuth(request);
        QNetworkReply *reply = get(request);
        if (reply) {
            reply->setProperty("isJson", jsonFile);
            reply->setProperty("endpoint", url);
        }
        else {
            mCallbacks.remove(key);
        }
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
        QString serverError;
        if (reply->isOpen()){
            // the same rawData is filled as it maybe usefull in callbacks in case of error
            rawData = reply->readAll();

            // Handle All Errors
            if (reply->property("isJson").isValid() && reply->property("isJson").value<bool>()) {
                const auto errdoc = QJsonDocument::fromJson(rawData);
                reply->setProperty("server_field_errors", errdoc.object());
                serverError = errdoc.toJson();

            } else {
                serverError = "The network reply is not json, raw data: " + rawData;
            }

        } else {
            serverError = "The network reply is not open for reading error";
        }

        qWarning() << "API ERROR (" << endpoint << ") # code: "<< reply->error() <<
            ", network error: " << reply->errorString() <<
            ", Server error: " << serverError.toStdString().c_str();

    } else {
        rawData = reply->readAll();

        // process The response, only if json, otherwise no need to even log! will be handled in callback if needed
        if (reply->property("isJson").isValid() && reply->property("isJson").value<bool>()) {

            data = prepareJsonResponse(endpoint, rawData);
            if (data.isEmpty() && reply->property("noDataObject").isValid() && reply->property("noDataObject").value<bool>())
                data = RestApiExecutor::prepareJsonResponse(endpoint, rawData);

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

    } else {
        TRACE << "Can not find the callback: " << key;
    }
}

QString RestApiExecutor::prepareUrlWithEmail(const QString &baseUrl, const QString &email) {
    QUrl url(baseUrl);

    QUrlQuery query;
    query.addQueryItem("email", QUrl::toPercentEncoding(email));

    url.setQuery(query);
    return url.toString();
}

QString RestApiExecutor::getReplyError(const QNetworkReply *reply) {
    QString err = reply->error() == QNetworkReply::UnknownContentError ? reply->property("server_field_errors").toJsonObject().value("message").toString() : "";

    if (err.isEmpty()) {
        err = reply->errorString();
        err.remove(reply->url().toString());
    }

    return err;
}

bool RestApiExecutor::isNeedRetryNetRequest(const QNetworkReply *reply) {
    auto replyError = reply->error();

    bool isNeedRetry = replyError != QNetworkReply::NoError &&
                       replyError != QNetworkReply::ContentAccessDenied &&
                       replyError != QNetworkReply::ContentOperationNotPermittedError &&
                       replyError != QNetworkReply::ContentNotFoundError &&
                       replyError != QNetworkReply::AuthenticationRequiredError &&
                       replyError != QNetworkReply::ContentReSendError &&
                       replyError != QNetworkReply::ContentConflictError &&
                       replyError != QNetworkReply::ContentGoneError &&
                       replyError != QNetworkReply::UnknownContentError;

    return isNeedRetry;
}

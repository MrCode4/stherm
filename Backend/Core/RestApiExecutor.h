#pragma once

#include "HttpExecutor.h"

/*! ***********************************************************************************************
 * Base class to manage REST API requests.
 * ************************************************************************************************/

class RestApiExecutor : public HttpExecutor
{
    Q_OBJECT
protected:
    using ResponseCallback = std::function<void (QNetworkReply* reply, const QByteArray& rawData, QJsonObject& data)>;

public:
    RestApiExecutor(QObject *parent = nullptr);

    QNetworkReply* callGetApi(const QString& endpoint, ResponseCallback callback = nullptr, bool setAuth = true);
    QNetworkReply* callPostApi(const QString& endpoint, const QByteArray &postData, ResponseCallback callback = nullptr, bool setAuth = true);
    QNetworkReply* callPutApi(const QString &endpoint, const QByteArray &postData, ResponseCallback callback, bool setAuth = true);
    QNetworkReply* downloadFile(const QString& url, ResponseCallback callback = nullptr, bool jsonFile = true, bool setAuth = false);

protected:
    QNetworkRequest prepareApiRequest(const QString &endpoint, bool setAuth = true);

    virtual void setApiAuth(QNetworkRequest& request);
    virtual QJsonObject prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const;

    void processNetworkReply(QNetworkReply *reply) override;
    bool errorHandled() override { return true; }

    //! Prepare URL with email
    //! convert email string to askii
    //! return baseUrl?email=encodedEmail as string.
    QString prepareUrlWithEmail(const QString &baseUrl, const QString &email);

    //! Extract and return an appropriate error message from the network reply.
    //! If a UnknownContentError occurred, retrieve the `message` from `the server_field_errors` field.
    //! Otherwise, return the general reply error message.
    QString getReplyError(const QNetworkReply *reply);

    //! A 2xx NetworkError status code signifies a successful request completion (exist server error).
    //! Therefore, retrying such requests is unnecessary and may lead to redundant operations.
    bool isNeedRetryNetRequest(const QNetworkReply *reply);

private:
    QString prepareHashKey(int operation, const QString& endpoint);

private:
    QHash <QString, ResponseCallback> mCallbacks;
};

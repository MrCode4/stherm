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
    virtual void setApiAuth(QNetworkRequest& request);
    QNetworkRequest prepareApiRequest(const QString& endpoint, bool setAuth = true);
    virtual QJsonObject prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const;
    void processNetworkReply(QNetworkReply* reply) override;

private:
    QString prepareHashKey(int operation, const QString& endpoint);

private:
    QHash <QString, ResponseCallback> mCallbacks;
};

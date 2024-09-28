#include "HttpExecutor.h"

#include "NetworkManager.h"

HttpExecutor::HttpExecutor(QObject *parent)
    : QObject(parent)
{
}

QNetworkReply* HttpExecutor::get(const QNetworkRequest& request)
{
    QNetworkReply* reply = NetworkManager::instance()->get(request);
    connect(reply, &QNetworkReply::finished, this,  &HttpExecutor::onRequestFinished);
    reply->ignoreSslErrors();
    return reply;
}

QNetworkReply* HttpExecutor::post(const QNetworkRequest& request, const QByteArray& data)
{
    QNetworkReply* reply = NetworkManager::instance()->post(request, data);
    connect(reply, &QNetworkReply::finished, this,  &HttpExecutor::onRequestFinished);
    reply->ignoreSslErrors();
    return reply;
}


void HttpExecutor::onRequestFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    processNetworkReply(reply);
    reply->deleteLater();
}

void HttpExecutor::processNetworkReply(QNetworkReply*) {}

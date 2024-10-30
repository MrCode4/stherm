#include "HttpExecutor.h"

#include "NetworkManager.h"

HttpExecutor::HttpExecutor(QObject *parent)
    : QObject(parent)
{
}

QNetworkReply* HttpExecutor::get(const QNetworkRequest& request)
{
    QNetworkReply *reply = NetworkManager::instance()->get(request, errorHandled());
    if (reply) {
        connect(reply, &QNetworkReply::finished, this,  &HttpExecutor::onRequestFinished);
        reply->ignoreSslErrors();
    }

    return reply;
}

QNetworkReply* HttpExecutor::post(const QNetworkRequest& request, const QByteArray& data)
{
    QNetworkReply *reply = NetworkManager::instance()->post(request, data, errorHandled());
    if (reply) {
        connect(reply, &QNetworkReply::finished, this,  &HttpExecutor::onRequestFinished);
        reply->ignoreSslErrors();
    }
    return reply;
}

QNetworkReply* HttpExecutor::put(const QNetworkRequest& request, const QByteArray& data)
{
    QNetworkReply *reply = NetworkManager::instance()->put(request, data, errorHandled());
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

void HttpExecutor::processNetworkReply(QNetworkReply *) {}

bool HttpExecutor::errorHandled()
{
    return false;
}

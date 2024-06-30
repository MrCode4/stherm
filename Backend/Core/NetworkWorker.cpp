#include "NetworkWorker.h"
#include "NetworkManager.h"

NetworkWorker::NetworkWorker(QObject *parent)
    : QObject(parent)
{
}

QNetworkReply* NetworkWorker::get(const QNetworkRequest& request)
{
    QNetworkReply* reply = NetworkManager::instance()->get(request);
    connect(reply, &QNetworkReply::finished, this,  &NetworkWorker::onRequestFinished);
    return reply;
}


QNetworkReply* NetworkWorker::post(const QNetworkRequest& request, const QByteArray& data)
{
    QNetworkReply* reply = NetworkManager::instance()->post(request, data);
    connect(reply, &QNetworkReply::finished, this,  &NetworkWorker::onRequestFinished);
    return reply;
}


void NetworkWorker::onRequestFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    processNetworkReply(reply);
    reply->deleteLater();
}

void NetworkWorker::processNetworkReply(QNetworkReply*) {}

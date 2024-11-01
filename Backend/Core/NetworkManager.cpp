#include "NetworkManager.h"

#include "LogHelper.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QAbstractNetworkCache>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
    mNetManager = new QNetworkAccessManager(this);
    connect(mNetManager, &QNetworkAccessManager::finished, this,  &NetworkManager::processNetworkReply);
}

NetworkManager* NetworkManager::sMe = nullptr;
NetworkManager* NetworkManager::instance()
{
    if (sMe == nullptr) {
        sMe = new NetworkManager(qApp);
    }
    return sMe;
}

void NetworkManager::clearCache()
{
    if (mNetManager->cache()){
        mNetManager->cache()->clear();
    }

    mNetManager->clearAccessCache();
    mNetManager->clearConnectionCache();
}

QNetworkReply *NetworkManager::get(const QNetworkRequest &request, bool noCheckError)
{
    auto reply = mNetManager->get(request);
    if (reply) reply->setProperty("noCheckError", noCheckError);
    return reply;
}

QNetworkReply *NetworkManager::post(const QNetworkRequest &request,
                                    const QByteArray &data,
                                    bool noCheckError)
{
    auto reply = mNetManager->post(request, data);
    if (reply) reply->setProperty("noCheckError", noCheckError);
    return reply;
}

QNetworkReply *NetworkManager::put(const QNetworkRequest &request,
                                   const QByteArray &data,
                                   bool noCheckError)
{
    auto reply = mNetManager->put(request, data);
    reply->setProperty("noCheckError", noCheckError);
    return reply;
}

void NetworkManager::processNetworkReply(QNetworkReply *netReply)
{
    bool noCheckError = netReply->property("noCheckError").toBool();
    // Handle All Errors
    if (!noCheckError && netReply->error() != QNetworkReply::NoError && netReply->isOpen()) {
        const auto reply = netReply->readAll();
        qWarning() << "network request finished with error:" << netReply->error()
                   << netReply->errorString() << reply;
        netReply->setProperty("errorReply", reply);
        setProperty("lastError", reply);
    }
}

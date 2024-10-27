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

QNetworkReply* NetworkManager::get(const QNetworkRequest& request)
{
    return isEnable() ? mNetManager->get(request) : nullptr;
}

QNetworkReply* NetworkManager::post(const QNetworkRequest& request, const QByteArray& data)
{
    return isEnable() ? mNetManager->post(request, data) : nullptr;
}

QNetworkReply* NetworkManager::put(const QNetworkRequest& request, const QByteArray& data)
{
    return mNetManager->put(request, data);
}

void NetworkManager::processNetworkReply(QNetworkReply *netReply)
{
}

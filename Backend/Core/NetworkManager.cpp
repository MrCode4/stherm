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
    return mNetManager->get(request);
}

QNetworkReply* NetworkManager::post(const QNetworkRequest& request, const QByteArray& data)
{
    return mNetManager->post(request, data);
}

void NetworkManager::processNetworkReply(QNetworkReply *netReply)
{
    // Handle All Errors
    if (netReply->error() != QNetworkReply::NoError && netReply->isOpen()) {
        qDebug() << Q_FUNC_INFO <<__LINE__<< netReply->error()<<netReply->errorString();
        const auto errdoc = QJsonDocument::fromJson(netReply->readAll());
        const QJsonObject errObj = errdoc.object();
        QStringList errMsg = errObj.value("non_field_errors").toVariant().toStringList();
        TRACE << errdoc.toJson().toStdString().c_str();
    }
}

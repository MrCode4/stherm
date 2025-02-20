#pragma once

#include <QObject>

#include "Property.h"


/*! ***********************************************************************************************
 * singletone class to manage network requests.
 * ************************************************************************************************/

class QNetworkRequest;
class QNetworkReply;
class QNetworkAccessManager;

class NetworkManager : public QObject
{
    Q_OBJECT
    NetworkManager(QObject* parent = nullptr);

    // If disabled, all the network calls will be ignored. We can use this when we need to block all
    // internet communications, e.g., when perf-test is running.
    PROPERTY_PUB_DEF_VAL(bool, isEnable, true)

public:
    static NetworkManager* instance();

    void clearCache();

    QNetworkReply *get(const QNetworkRequest &request, bool noCheckError = false);
    QNetworkReply *post(const QNetworkRequest &request,
                        const QByteArray &data,
                        bool noCheckError = false);
    QNetworkReply *put(const QNetworkRequest &request,
                       const QByteArray &data,
                       bool noCheckError = false);

signals:
    void cacheCleared();


private slots:
    void processNetworkReply(QNetworkReply *netReply);

private:
    static NetworkManager* sMe;
    QNetworkAccessManager *mNetManager;
};

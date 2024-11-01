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

private slots:
    void processNetworkReply(QNetworkReply *netReply);

private:
    static NetworkManager* sMe;
    QNetworkAccessManager *mNetManager;
};

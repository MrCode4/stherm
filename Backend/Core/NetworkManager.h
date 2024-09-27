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
    PROPERTY_PUB_DEF_VAL(bool, isEnable, true)
public:
    static NetworkManager* instance();

    void clearCache();

    QNetworkReply* get(const QNetworkRequest& request);
    QNetworkReply* post(const QNetworkRequest& request, const QByteArray& data);
    QNetworkReply* put(const QNetworkRequest& request, const QByteArray& data);

private slots:
    void processNetworkReply(QNetworkReply *netReply);

private:
    static NetworkManager* sMe;
    QNetworkAccessManager *mNetManager;
};

#pragma once

#include <QObject>


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

    QNetworkReply* get(const QNetworkRequest& request);
    QNetworkReply* post(const QNetworkRequest& request, const QByteArray& data);
    QNetworkReply* put(const QNetworkRequest& request, const QByteArray& data);

private slots:
    void processNetworkReply(QNetworkReply *netReply);

private:
    static NetworkManager* sMe;
    QNetworkAccessManager *mNetManager;
};

#pragma once

#include <QObject>
#include <QtNetwork>

/*! ***********************************************************************************************
 * Base class to manage network requests.
 * ************************************************************************************************/

class HttpExecutor : public QObject
{
    Q_OBJECT

public:
    HttpExecutor(QObject *parent = nullptr);

    QNetworkReply* get(const QNetworkRequest& request);
    QNetworkReply* post(const QNetworkRequest& request, const QByteArray& data);
    QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data);

protected:
    virtual void processNetworkReply(QNetworkReply* reply);

private slots:
    void onRequestFinished();
};

#pragma once

#include <QObject>
#include <QtNetwork>

/*! ***********************************************************************************************
 * Base class to manage network requests.
 * ************************************************************************************************/

constexpr char m_methodProperty[] = "method";

class HttpExecutor : public QObject
{
    Q_OBJECT

public:
    HttpExecutor(QObject *parent = nullptr);

    QNetworkReply* get(const QNetworkRequest& request);
    QNetworkReply* post(const QNetworkRequest& request, const QByteArray& data);

protected:
    virtual void processNetworkReply(QNetworkReply* reply);

private slots:
    void onRequestFinished();
};

#pragma once

#include <QObject>
#include <QtNetwork>

/*! ***********************************************************************************************
 * Interface class to manage network requests.
 * ************************************************************************************************/

constexpr char m_methodProperty[] = "method";

class NetworkWorker : public QObject
{
    Q_OBJECT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/

    NetworkWorker(QObject *parent = nullptr);

    /* Public Functions
     * ****************************************************************************************/

    //! Prepare post request data
    virtual QByteArray preparePacket(QString className, QString method, QJsonArray params);

    //! Send post request
    virtual void sendPostRequest(const QUrl &mainUrl, const QUrl &relativeUrl,
                                 const QByteArray &postData, const QString &method = QString());


protected slots:
    //! Process network replay
    virtual void processNetworkReply(QNetworkReply *netReply);

protected:
     QNetworkAccessManager *mNetManager;
};

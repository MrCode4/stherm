#pragma once

#include <QObject>
#include <QtNetwork>

class System : public QObject
{
    Q_OBJECT
public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/

    System(QObject *parent = nullptr);

    /* Public Functions (Setters and getters)
     * ****************************************************************************************/

    //! Reboot device
    void rebootDevice();

    void getQR();
private slots:
    void processNetworkReply(QNetworkReply *netReply);
private:
    void sendPostRequest(const QUrl &relativeUrl, const QByteArray &postData);

    /* Attributes
     * ****************************************************************************************/
    QNetworkAccessManager *mNetManager;
};

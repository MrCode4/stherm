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

    //! Get technic's url and serial number
    void getQR(QString accessUid);

    //! Get update
    //! todo: process response packet
    //! TEMP: "022"
    void getUpdate(QString softwareVersion = "022");

private slots:
    void processNetworkReply(QNetworkReply *netReply);

private:
    void sendPostRequest(const QUrl &relativeUrl, const QByteArray &postData);

    /* Attributes
     * ****************************************************************************************/
    QNetworkAccessManager *mNetManager;

    QString mSerialNumber;
};

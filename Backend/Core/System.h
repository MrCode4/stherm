#pragma once

#include <QObject>
#include <QtNetwork>

/*! ***********************************************************************************************
 * This class manage system requests.
 * ************************************************************************************************/

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

    //! Send request job to web server
    void requestJob(QString type);

private slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

private:
    //! Prepare post request data
    QByteArray preparePacket(QString className, QString method, QJsonArray params);

    //! Send post request
    void sendPostRequest(const QUrl &relativeUrl, const QByteArray &postData);

    /* Attributes
     * ****************************************************************************************/
    QNetworkAccessManager *mNetManager;

    QString mSerialNumber;
};

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
};

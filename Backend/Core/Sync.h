#pragma once

#include <QObject>
#include <QtNetwork>

#include "NetWorkWorker.h"

/*! ***********************************************************************************************
 * This class manage sync requests.
 * ************************************************************************************************/

class Sync : public NetworkWorker
{
    Q_OBJECT

public:
    Sync(QObject *parent = nullptr);

    void changeContractorInfo(QString serialNumber);

private slots:
    //! Process network replay
    void processNetworkReply(QNetworkReply *netReply);

private:
    /* Attributes
     * ****************************************************************************************/
};

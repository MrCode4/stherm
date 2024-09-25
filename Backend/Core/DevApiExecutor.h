#pragma once

#include "RestApiExecutor.h"

/*! ***********************************************************************************************
 * Base class to manage Device related REST API requests.
 * ************************************************************************************************/

class DevApiExecutor : public RestApiExecutor
{
    Q_OBJECT
public:
    DevApiExecutor(QObject *parent = nullptr);

protected:
    virtual void setApiAuth(QNetworkRequest& request) override;
    QNetworkRequest prepareApiRequest(const QString& endpoint, bool setAuth = true);
    QJsonObject prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const override;
};

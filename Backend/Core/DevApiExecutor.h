#pragma once

#include "RestApiExecutor.h"
#include "Property.h"

/*! ***********************************************************************************************
 * Base class to manage Device related REST API requests.
 * ************************************************************************************************/

class DevApiExecutor : public RestApiExecutor
{
    Q_OBJECT
    PROPERTY_PRI(QString, baseUrl)
public:
    DevApiExecutor(QObject *parent = nullptr);

protected:
    virtual void setApiAuth(QNetworkRequest& request) override;
    QNetworkRequest prepareApiRequest(const QString& endpoint, bool setAuth = true);
    QJsonObject prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const override;
};

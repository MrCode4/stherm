#include "DevApiExecutor.h"
#include "DeviceInfo.h"
#include "LogHelper.h"

DevApiExecutor::DevApiExecutor(QObject *parent)
    : RestApiExecutor(parent)
{
}

void DevApiExecutor::setApiAuth(QNetworkRequest& request)
{
    RestApiExecutor::setApiAuth(request);
    auto authData = Device->uid() + Device->serialNumber().toStdString();
    // Get error: QNetworkReply::ProtocolFailure "Server is unable to maintain
    // the header compression context for the connection"
    request.setRawHeader("Authorization", "Bearer " + QCryptographicHash::hash(authData, QCryptographicHash::Sha256).toHex());
}

QJsonObject DevApiExecutor::prepareJsonResponse(const QString& endpoint, const QByteArray& rawData) const
{
    QJsonObject data;
    const QJsonObject rootObject = RestApiExecutor::prepareJsonResponse(endpoint, rawData);

    if (rootObject.contains("data")) {
        data = rootObject.value("data").toObject();
    }
    else {
        TRACE << "API ERROR (" << endpoint << ") : " << " Reponse contains no data object";
    }

    return data;
}

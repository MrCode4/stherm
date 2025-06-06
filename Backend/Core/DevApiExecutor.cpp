#include "DevApiExecutor.h"
#include "DeviceInfo.h"
#include "LogHelper.h"
#include "Config.h"

DevApiExecutor::DevApiExecutor(QObject *parent)
    : RestApiExecutor(parent)
{
    auto url = qEnvironmentVariable("API_SERVER_BASE_URL", API_SERVER_BASE_URL);
    if (!url.endsWith('/')) {
        qWarning() << "sync base url is not valid" << url;
        url += '/';
    }

    baseUrl(url);
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
        if (rootObject.value("data").isObject())
            data = rootObject.value("data").toObject();

        if (rootObject.value("data").isArray()) {
            TRACE << endpoint << " return an array...";
            data = rootObject;
        }
    }
    else {
        TRACE << "API ERROR (" << endpoint << ") : " << " Reponse contains no data object";
    }

    return data;
}

#include "ProtoDataManagerCPP.h"
#include "DeviceInfo.h"
#include "LogHelper.h"

ProtoDataManagerCPP::ProtoDataManagerCPP(QObject *parent)
    : DevApiExecutor{parent}
{

}

void ProtoDataManagerCPP::updateData() {
    auto callback = [this](QNetworkReply* reply, const QByteArray &rawData, QJsonObject &data) {
        TRACE << "test: " << reply->errorString();

    };
    QByteArray serializedData;
    QFile file("/usr/local/bin/response2.bin");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        serializedData = file.readAll();
        file.close();
    } else {
        qWarning() << "Error opening file: " << file.errorString();
        return;
    }

    QString sn = Device->serialNumber();
    auto url = baseUrl() + QString("api/monitor/data?sn=%0").arg(sn);
    callPostApi(url, serializedData, callback, true, "application/x-protobuf");
}

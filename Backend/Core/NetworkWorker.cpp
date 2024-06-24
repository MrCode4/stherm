#include "NetworkWorker.h"
#include "LogHelper.h"
#include "NetworkManager.h"

NetworkWorker::NetworkWorker(QObject *parent) : QObject(parent)
{

}

QByteArray NetworkWorker::preparePacket(QString className, QString method, QJsonArray params)
{
    QJsonObject requestData;
    requestData["request"] = QJsonObject{
        {"class", className},
        {"method", method},
        {"params", params}
    };

    requestData["user"] = QJsonObject{
        {"lang_id", 0},
        {"user_id", 0},
        {"type_id", 0},
        {"host_id", 0},
        {"region_id", 0},
        {"token", ""}
    };

    QJsonDocument jsonDocument(requestData);

    return jsonDocument.toJson();
}

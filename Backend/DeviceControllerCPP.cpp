#include "DeviceControllerCPP.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include "UtilityHelper.h"


/* ************************************************************************************************
 * Constructors & Destructor
 * ************************************************************************************************/
DeviceControllerCPP::DeviceControllerCPP(QObject *parent)
{
    _mainData = {{"temp", QVariant(0)}, {"hum", QVariant(0)}};

    mDataParser = new DataParser();

    // Get data from parser and pass to UI
    connect(mDataParser, &DataParser::dataReay, this, [=](QVariantMap data) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "DataParser:   " << data;
        _mainData = data;

    });


}

DeviceControllerCPP::~DeviceControllerCPP()
{
}

void DeviceControllerCPP::createSensor(QString name, QString id)
{

}

QVariantMap DeviceControllerCPP::sendRequest(QString className, QString method, QVariantList data)
{
    mDataParser->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
    mDataParser->sendRequest(STHERM::SIOCommand::GetSensors, STHERM::PacketType::UARTPacket);
    mDataParser->sendRequest(STHERM::SIOCommand::GetTOF, STHERM::PacketType::UARTPacket);

    if (className == "system") {
        if (method == "getMainData") {
            return getMainData();
        }
    }

    qDebug() << "Request received: " << className << method << data;

    if (className == "hardware") {
        if (method == "setSettings") {
            if (data.size() < 0) {
                qWarning() << "data sent is empty";
            } else {
                if (data.size() != 6) {
                    qWarning() << "data sent is not consistent";
                }
                setBrightness(std::clamp(qRound(data.first().toDouble()), 0, 254));
            }
        }
    }

    return {};
}

int DeviceControllerCPP::getStartMode(int pinNumber)
{
    return UtilityHelper::getStartMode(pinNumber);
}

QString DeviceControllerCPP::getCPUInfo()
{
    return UtilityHelper::getCPUInfo();
}

void DeviceControllerCPP::setBrightness(int value)
{
    UtilityHelper::setBrightness(value);
}

void DeviceControllerCPP::setTimeZone(int offset)
{
    UtilityHelper::setTimeZone(offset);
}


QVariantMap DeviceControllerCPP::getMainData()
{
    return _mainData;
}

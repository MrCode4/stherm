#include "UtilityHelper.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>

// Definitions of special characters used in serial communication
#define phyStart        0xF0
#define phyStop         0xF1
#define phyCtrlEsc      0xF2
#define phyXorByte      0xFF
#define POLY 0x8408

// Minimum packet length
#define PacketMinLength 5

bool UtilityHelper::configurePins(int gpio)
{
    // Update export file
    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open export file.";
        return false;
    }

    // Convert pinNumber to string
    QString pinString = QString::number(gpio);

    // Write the pin number to the export file
    QTextStream out(&exportFile);
    out << pinString;
    exportFile.close();


    // Update direction file
    QString directionFilePath = QString("/sys/class/gpio/gpio%0/direction").arg(gpio);
    QFile directionFile(directionFilePath);

    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open direction file for pin " << gpio;
        return false;
    }

    QTextStream outIn(&directionFile);
    outIn << "in";

    directionFile.close();

    // Update edge file
    QString edgeFilePath = QString("/sys/class/gpio/gpio%d/edge").arg(gpio);
    QFile edgeFile(directionFilePath);

    if (!edgeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open eadge file for pin " << gpio;
        return false;
    }

    QTextStream outInEdge(&edgeFile);
    outIn << "falling";

    edgeFile.close();

    return true;
}

void UtilityHelper::exportGPIOPin(int pinNumber)
{
    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open export file.";
        return;
    }

    // Convert pinNumber to string
    QString pinString = QString::number(pinNumber);

    // Write the pin number to the export file
    QTextStream out(&exportFile);
    out << pinString;
    exportFile.close();


    QString directionFilePath = QString("/sys/class/gpio/gpio%0/direction").arg(pinNumber);
    QFile directionFile(directionFilePath);

    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open direction file for pin " << pinNumber;
        return;
    }

    QTextStream outIn(&directionFile);
    outIn << "in";

    directionFile.close();
}

int UtilityHelper::getStartMode(int pinNumber)
{
    exportGPIOPin(pinNumber);

    // Define the file path
    QString filePath = QString("/sys/class/gpio/gpio%0/value").arg(QString::number(pinNumber));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open the file.";
        return -1;
    }

    QTextStream in(&file);
    QString value;
    in >> value; // Read the content of the file

    qDebug() << Q_FUNC_INFO << __LINE__ << value;

    file.close();

    int result = (value.trimmed() == "0") ? 1 : 0;
    return result;
}

QString UtilityHelper::getCPUInfo() {
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ <<"Failed to open the file.";
        return NULL;
    }

    QString cpuInfo;
    QString serialNumberHex;
    QString line = file.readLine();

    while (!line.isEmpty()) {
        cpuInfo.append(line).append('\n');

        if (line.startsWith("Serial")) {
            QRegularExpression re("Serial\\s*:\\s*([A-Fa-f0-9]+)");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                serialNumberHex = match.captured(1).simplified();
                break;
            }
        }
        line = file.readLine();
    }

    file.close();

    //    qDebug() << Q_FUNC_INFO << __LINE__ << "cpuInfo: " << cpuInfo;
    //    qDebug() << Q_FUNC_INFO << __LINE__ << "Serial Number: " << serialNumberHex;
    return serialNumberHex;
}

bool UtilityHelper::setBrightness(int value) {
    QFile brightnessFile("/sys/class/backlight/backlight_display/brightness");
    if (!brightnessFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__  << "Failed to open brightness file.";
        return false;
    }

    QTextStream out(&brightnessFile);
    out << QString::number(value); // Write the desired brightness value
    brightnessFile.close();

    qDebug() << "Brightness set successfully!";
    return true;
}

void UtilityHelper::setTimeZone(int offset) {
    QString timezoneName = "GMT";
    QString offsetStr = (offset >= 0) ? "+" : "";
    offsetStr += QString::number(offset);

    QString timezonePath = "/usr/share/zoneinfo/Etc/";
    QString linkPath = "/etc/localtime";

    QString timezoneFile = timezonePath + timezoneName + offsetStr;

    if (!QFile::exists(timezoneFile)) {
        qDebug() << Q_FUNC_INFO << __LINE__  << "Timezone file not found.";
        return;
    }

    int exitCode = QProcess::execute("ln", {"-sf", timezoneFile, linkPath});

    if (exitCode >= 0) {
        qDebug() << Q_FUNC_INFO << __LINE__  << "Timezone set to" << timezoneFile;
    }
}


uint16_t UtilityHelper::setSIOTxPacket(uint8_t *TxDataBuf, STHERM::SIOPacket TxPacket) {
    uint8_t tmpTxBuffer[256];
    uint16_t index = 0;
    uint16_t packetLen = 0;

    tmpTxBuffer[0] = TxPacket.CMD;
    tmpTxBuffer[1] = TxPacket.ACK;
    tmpTxBuffer[2] = TxPacket.SID;
    index += 3;
    memcpy(&tmpTxBuffer[index], &TxPacket.DataArray[0], TxPacket.DataLen);
    index += TxPacket.DataLen;
    TxPacket.CRC = crc16(&TxPacket.DataArray[0], TxPacket.DataLen);
    memcpy(&tmpTxBuffer[index], &TxPacket.CRC, 2);
    index += 2;

    TxDataBuf[packetLen++] = phyStart;

    int a = 0;
    for(; a < index; a++)
    {
        if ( (tmpTxBuffer[a] == phyStart) || (tmpTxBuffer[a] == phyStop) || (tmpTxBuffer[a] == phyCtrlEsc) )
        {
            TxDataBuf[packetLen++] = phyCtrlEsc;
            TxDataBuf[packetLen++] = tmpTxBuffer[a] ^ phyXorByte;
        }
        else {
            TxDataBuf[packetLen++] = tmpTxBuffer[a];
        }
    }
    TxDataBuf[packetLen++] = phyStop;

    return packetLen;
}

bool UtilityHelper::SerialDataRx(uint8_t RxData, STHERM::SerialRxData *RxDataCfg) {
    switch (RxData) {
    case phyStart: {
        RxDataCfg->RxDataLen = 0;
        RxDataCfg->RxActive = true;
        RxDataCfg->RxPacketDone = false;
        RxDataCfg->RxCtrlEsc = false;
    } break;
    case phyStop: {
        RxDataCfg->RxActive = false;
        RxDataCfg->RxPacketDone = true;
        RxDataCfg->RxCtrlEsc = false;
    } break;
    case phyCtrlEsc: {
        RxDataCfg->RxCtrlEsc = true;
    } break;
    default: {
        if (RxDataCfg->RxActive == true) {
            if (RxDataCfg->RxCtrlEsc == true) {
                RxDataCfg->RxCtrlEsc = false;
                RxDataCfg->RxDataArray[RxDataCfg->RxDataLen] = phyXorByte ^ RxData;
            } else {
                RxDataCfg->RxDataArray[RxDataCfg->RxDataLen] = RxData;
            }
            RxDataCfg->RxDataLen++;
        }
    } break;
    }
    if (RxDataCfg->RxPacketDone == true) {
        RxDataCfg->RxPacketDone = false;
        if (RxDataCfg->RxDataLen >= PacketMinLength) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

unsigned short UtilityHelper::crc16(unsigned char *data_p, unsigned short length)
{
    unsigned char i;
    unsigned int data;
    unsigned int crc = 0xffff;

    if (length == 0)
        return (~crc);

    do
    {
        for (i = 0, data = (unsigned int)0xff & *data_p++;
             i < 8;
             i++, data >>= 1)
        {
            if ((crc & 0x0001) ^ (data & 0x0001))
                crc = (crc >> 1) ^ POLY;
            else  crc >>= 1;
        }
    } while (--length);

    crc = ~crc;
    data = crc;
    crc = (crc << 8) | (data >> 8 & 0xff);

    return (crc);
}

uint8_t UtilityHelper::packetType(STHERM::PacketType packetType) {
    switch (packetType) {
    case STHERM::PacketType::NONEPacket:
        return NONE_Packet;

    case STHERM::PacketType::UARTPacket:
        return UART_Packet;

    case STHERM::PacketType::NUSPacket:
        return NUS_Packet;

    default:
        return NONE_Packet;
    }

    return NONE_Packet;
}

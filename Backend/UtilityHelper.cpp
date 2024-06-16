#include "UtilityHelper.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>
#include <QEventLoop>
#include <QTimer>

#include "LogHelper.h"

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
    TRACE << gpio;

    // Update export file
    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        TRACE << "Failed to open export file.";
        return false;
    }

    // Convert pinNumber to string
    QString pinString = QString::number(gpio);

    // Write the pin number to the export file
    QTextStream out(&exportFile);
    out << pinString;
    exportFile.close();

    // Sleep for 1100 ms
    QEventLoop loop;
    QTimer timer;
    timer.connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    // Update direction file
    QString directionFilePath = QString("/sys/class/gpio/gpio%0/direction").arg(gpio);
    QFile directionFile(directionFilePath);

    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        TRACE << "Failed to open direction file for pin " << gpio;
        return false;
    }

    timer.start(1100);
    loop.exec();

    QTextStream outIn(&directionFile);
    outIn << "in";

    directionFile.close();

    // Update edge file
    QString edgeFilePath = QString("/sys/class/gpio/gpio%0/edge").arg(gpio);
    QFile edgeFile(edgeFilePath);

    if (!edgeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        TRACE << "Failed to open eadge file for pin " << gpio;
        return false;
    }

    QTextStream outInEdge(&edgeFile);
    outInEdge << "falling";

    edgeFile.close();

    TRACE << gpio << "successful";
    return true;
}

void UtilityHelper::exportGPIOPin(int pinNumber, bool isOutput)
{
    TRACE << pinNumber;

    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        TRACE << "Failed to open export file.";
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
        TRACE << "Failed to open direction file for pin " << pinNumber;
        return;
    }

    QTextStream outIn(&directionFile);
    if (isOutput) {
        outIn << "out";
    }
    else {
        outIn << "in";
    }

    directionFile.close();
}

int UtilityHelper::getGpioValue(int pinNumber)
{
    exportGPIOPin(pinNumber, false);

    // Define the file path
    QString filePath = QString("/sys/class/gpio/gpio%0/value").arg(QString::number(pinNumber));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        TRACE << "Failed to open the file. The pin(" << pinNumber << ") value could not be opened";
        return -1;
    }

    QTextStream in(&file);
    QString value;
    in >> value; // Read the content of the file

    TRACE << value;

    file.close();

    int result = (value.trimmed() == "0") ? 1 : 0;
    return result;
}

std::string convert_hex_to_string(const std::string& hex_value) {

#ifdef __unix__
    // Remove the "0x" prefix (if present)
    std::string value = hex_value.substr(2);

    // Convert the hex string to a long long int (assuming 64-bit system)
    std::stringstream ss;
    ss << std::hex << value;
    unsigned long long int integer_value;
    ss >> integer_value;

    // Format the integer as an 8-character string with leading zeros
    std::stringstream formatted_string;
    formatted_string << std::setfill('0') << std::setw(8) << std::hex << integer_value;

    return formatted_string.str();
#endif

    return hex_value;
}

QString UtilityHelper::getCPUInfo()
{
    QFile file("/sys/fsl_otp/HW_OCOTP_CFG1");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        TRACE << "Failed to open the CFG1 file.";
        return NULL;
    }
    auto line = file.readLine();
    file.close();

    if (!line.startsWith("0x") || !line.endsWith("\n"))
    {
        TRACE << "CFG1 not validated" << line;
        return NULL;
    }

    line = line.trimmed();
    TRACE << "CFG1:" << line;
    QString cfg1 = QString::fromStdString(convert_hex_to_string(line.toStdString()));

    QFile file0("/sys/fsl_otp/HW_OCOTP_CFG0");
    if (!file0.open(QIODevice::ReadOnly | QIODevice::Text)) {
        TRACE << "Failed to open the CFG0 file.";
        return NULL;
    }
    line = file0.readLine();
    file0.close();

    if (!line.startsWith("0x") || !line.endsWith("\n"))
    {
        TRACE << "CFG0 not validated:" << line;
        return NULL;
    }

    line = line.trimmed();
    TRACE << "CFG0:" << line;
    QString cfg0 = QString::fromStdString(convert_hex_to_string(line.toStdString()));

    QString serialNumberHex = cfg1 + cfg0;

//    TRACE << "Serial Number: " << serialNumberHex << cfg1 << cfg0;
    return serialNumberHex;
}

QString UtilityHelper::getCPUInfoOld()
{
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        TRACE << "Failed to open the file.";
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

    // TRACE << "cpuInfo: " << cpuInfo;
    // TRACE << "Serial Number: " << serialNumberHex;
    return serialNumberHex;
}

bool UtilityHelper::setBrightness(int value) {
#ifdef __unix__

    QFile brightnessFile("/sys/class/backlight/backlight_display/brightness");
    if (!brightnessFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        TRACE << "Failed to open brightness file.";
        return false;
    }

    QTextStream out(&brightnessFile);
    out << QString::number(value); // Write the desired brightness value
    brightnessFile.close();

    TRACE << "Brightness set successfully!" << value;

#endif
    return true;
}

int UtilityHelper::brightness() {
    QFile brightnessFile("/sys/class/backlight/backlight_display/brightness");
    if (!brightnessFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        TRACE << "Failed to open brightness file.";
        return -1;
    }

    bool isOk;
    auto brightnessValue = brightnessFile.readLine().toInt(&isOk);
    if (!isOk)
        TRACE << "Failed to read brightness value";

    return (isOk ? brightnessValue : -1);
}

void UtilityHelper::setTimeZone(int offset) {
    QString timezoneName = "GMT";
    QString offsetStr = (offset >= 0) ? "+" : "";
    offsetStr += QString::number(offset);

    QString timezonePath = "/usr/share/zoneinfo/Etc/";
    QString linkPath = "/etc/localtime";

    QString timezoneFile = timezonePath + timezoneName + offsetStr;

    if (!QFile::exists(timezoneFile)) {
        TRACE << "Timezone file not found.";
        return;
    }

    int exitCode = QProcess::execute("ln", {"-sf", timezoneFile, linkPath});

    if (exitCode >= 0) {
        TRACE << "Timezone set to" << timezoneFile;
    }
}

// TODO this takes an input packet and creates an escaped UART frame for sending
uint16_t UtilityHelper::setSIOTxPacket(uint8_t *TxDataBuf, STHERM::SIOPacket TxPacket) {
    uint8_t tmpTxBuffer[256];
    uint16_t index = 0;
    uint16_t packetLen = 0;

    tmpTxBuffer[index++] = TxPacket.CMD;
    tmpTxBuffer[index++] = TxPacket.ACK;
    tmpTxBuffer[index++] = TxPacket.SID;
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

    // Check the received byte:
    switch (RxData) {
    case phyStart: { // Initialize data reception for a new packet.
        RxDataCfg->RxDataLen = 0;
        RxDataCfg->RxActive = true;
        RxDataCfg->RxPacketDone = false;
        RxDataCfg->RxCtrlEsc = false;
    } break;

    case phyStop: {  // Mark the end of data for the current packet.
        RxDataCfg->RxActive = false;
        RxDataCfg->RxPacketDone = true;
        RxDataCfg->RxCtrlEsc = false;
    } break;

    case phyCtrlEsc: { // Handle control escape character, if needed for packet processing.
        RxDataCfg->RxCtrlEsc = true;
    } break;

    default: { // Process regular data byte received
        if (RxDataCfg->RxActive == true) {
            if (RxDataCfg->RxCtrlEsc == true) { // Handle escaped character.
                RxDataCfg->RxCtrlEsc = false;
                RxDataCfg->RxDataArray[RxDataCfg->RxDataLen] = phyXorByte ^ RxData;
            } else { // Store the received data byte in the packet data array.
                RxDataCfg->RxDataArray[RxDataCfg->RxDataLen] = RxData;
            }
            RxDataCfg->RxDataLen++;
        }
    } break;

    }

    // Check if the complete packet has been received and processed.
    if (RxDataCfg->RxPacketDone == true) {
        RxDataCfg->RxPacketDone = false;

        // Check if the received packet meets the minimum length of packet criteria.
        if (RxDataCfg->RxDataLen >= PacketMinLength) {
            return true;
        } else {
            return false;
        }
    } else { // Return false if the packet is not yet complete.
        return false;
    }
}

//! the calculated CRC-16 checksum
unsigned short UtilityHelper::crc16(unsigned char *data_p, unsigned short length)
{
    unsigned char i;
    unsigned int data;
    unsigned int crc = 0xffff;

    // If the length is zero, return the complement of the initial CRC value
    if (length == 0)
        return (~crc);

    // Loop through each byte of the data array
    do {
        // Process each bit of the current byte
        for (i = 0, data = (unsigned int)0xff & *data_p++;
             i < 8;
             i++, data >>= 1) {

            // XOR the least significant bits of CRC and data if necessary
            if ((crc & 0x0001) ^ (data & 0x0001))
                crc = (crc >> 1) ^ POLY;
            else
                crc >>= 1;
        }
    } while (--length); // Continue until all bytes are processed

    // Invert the bits of the final CRC value
    crc = ~crc; 
    data = crc;

    // Swap the bytes of the CRC
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

double UtilityHelper::CPUUsage() {
    #ifdef __unix__
    // Open /proc/stat file
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1; // Error opening file

    // Read all lines from /proc/stat
    QTextStream in(&file);
    QString line;

    // Read all data from file and split them
    auto fileLines = in.readAll().split("\n");

    foreach (auto var, fileLines) {
        // overall CPU statistics
        // Find line starting with "cpu "
        if (var.startsWith("cpu  ")) {
            line = var;
            break;
        }
    }

    // Close the file
    file.close();

    line.remove("cpu  ");
    // Parse CPU stats
    QStringList parts = line.split(" ");
    if (parts.size() < 5) {
        TRACE << "Insufficient data" << parts.size();
        return -1; // Insufficient data
    }

    double idle = parts[3].toDouble();
    double total = 0;
    for (int i = 1; i < parts.size(); ++i)
        total += parts[i].toDouble();

    // Calculate CPU usage percentage
    double usage = 100.0 * (1.0 - idle / total);

    TRACE << " CPU usage percentage: " << usage;

    return usage;

    #endif

    return -1;
}

QString STHERM::printModeStr(RelayMode mode) {
    return mode == ON ? "On" : (mode == NoWire ? "invalid" : "Off");
}

std::vector<std::pair<std::string, int> > STHERM::RelayConfigs::changeStepsSorted(const RelayConfigs &newState) {
    std::vector<std::pair<std::string, int>> transitions;
    auto factor = [](RelayMode current, RelayMode next, int factor) {
        int change = current == next ? 0 : (next == OFF ? -1 : 1);
        return change * factor;
    };

    transitions.push_back({"o/b", factor(o_b, newState.o_b, 1)});
    transitions.push_back({"g", factor(g, newState.g, 2)});
    transitions.push_back({"y1", factor(y1, newState.y1, 3)});
    transitions.push_back({"y2", factor(y2, newState.y2, 4)});
    transitions.push_back({"w1", factor(w1, newState.w1, 3)});
    transitions.push_back({"w2", factor(w2, newState.w2, 4)});
    transitions.push_back({"w3", factor(w3, newState.w3, 5)});

    // CHECK ORDER
    transitions.push_back({"acc2", factor(acc2, newState.acc2, 6)});
    transitions.push_back({"acc1p", factor(acc1p, newState.acc1p, 7)});
    transitions.push_back({"acc1n", factor(acc1n, newState.acc1n, 8)});

    std::sort(transitions.begin(), transitions.end(), [&](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
        return a.second < b.second;
    });

    return transitions;
}

QString STHERM::RelayConfigs::printStr(){
    return QString("o/b:%0, g:%1, y1:%2, y2:%3, w1:%4, w2:%5, w3:%6, acc2:%7, acc1n:%8, acc1p:%9").
        arg(printModeStr(o_b),printModeStr(g),printModeStr(y1),printModeStr(y2),printModeStr(w1),printModeStr(w2),printModeStr(w3),
            printModeStr(acc2), printModeStr(acc1n), printModeStr(acc1p));
}


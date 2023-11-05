#include "DeviceControllerCPP.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>


/* ************************************************************************************************
 * Constructors & Destructor
 * ************************************************************************************************/
DeviceControllerCPP::DeviceControllerCPP(QObject *parent)
{

}

DeviceControllerCPP::~DeviceControllerCPP()
{
}

void DeviceControllerCPP::createSensor(QString name, QString id)
{

}

void DeviceControllerCPP::sendRequest(QString className, QString method, QVariantList data)
{

}



void DeviceControllerCPP::exportGPIOPin(int pinNumber) {
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


int DeviceControllerCPP::getStartMode (int pinNumber) {
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

//! Get CPU info
QString DeviceControllerCPP::getCPUInfo() {
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

//! setBrightness, value is a number between 0 and 254
void DeviceControllerCPP::setBrightness(int value) {
    QFile brightnessFile("/sys/class/backlight/backlight_display/brightness");
    if (!brightnessFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__  << "Failed to open brightness file.";
        return;
    }

    QTextStream out(&brightnessFile);
    out << QString::number(value); // Write the desired brightness value
    brightnessFile.close();

    qDebug() << "Brightness set successfully!";
}

//! Set time zone
void DeviceControllerCPP::setTimeZone(int offset) {
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

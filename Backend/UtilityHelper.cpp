#include "UtilityHelper.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

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

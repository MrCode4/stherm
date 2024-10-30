#include "AppSpecCPP.h"

#include <QQmlEngine>
#include <QFile>
#include <sstream>

#include "device_config.h"

AppSpecCPP::AppSpecCPP(QObject *parent)
    : QObject{parent}
{
    this->mInstance = this;   // this class is intialized in qml
}

AppSpecCPP *AppSpecCPP::mInstance = nullptr;
AppSpecCPP *AppSpecCPP::instance()
{
    return mInstance;
}

QString AppSpecCPP::systemTypeString(SystemType systemType) {
    switch (systemType) {
    case Conventional:
        return "traditional";

    case HeatPump:
        return "heat_pump";

    case CoolingOnly:
        return "cooling";

    case HeatingOnly:
        return "heating";

    case DualFuelHeating:
        return "dual_fuel_heating";

    default:
        break;
    }

    return QString("traditional");
}

AppSpecCPP::SystemType AppSpecCPP::systemTypeToEnum(QString systemTypeStr) {
    if (systemTypeStr == "traditional") {
        return Conventional;
        
    } else if (systemTypeStr == "heat_pump") {
        return HeatPump;
        
    } else if (systemTypeStr == "cooling") {
        return CoolingOnly;

    } else if (systemTypeStr == "heating") {
        return HeatingOnly;

    } else if (systemTypeStr == "dual_fuel_heating") {
        return DualFuelHeating;
    }

    return Conventional;
}

QString AppSpecCPP::accessoriesWireTypeString(AccessoriesWireType wt)
{
    switch (wt) {
    case T1PWRD:
        return "T1PWRD";

    case T1Short:
        return "T1Short";

    case T2PWRD:
        return "T2PWRD";

    case None:
        return "None";

    default:
        break;
    }

    return QString("None");
}

AppSpecCPP::AccessoriesWireType AppSpecCPP::accessoriesWireTypeToEnum(QString wtStr)
{
    if (wtStr == "T1PWRD") {
        return T1PWRD;

    } else if (wtStr == "T1Short") {
        return T1Short;

    } else if (wtStr == "T2PWRD") {
        return T2PWRD;

    } else if (wtStr == "None") {
        return None;
    }

    return None;
}

QVariant AppSpecCPP::readFromFile(const QString& fileUrl)
{
    QFile file(fileUrl);
    if (file.open(QFile::ReadOnly)) {
        auto fileContent = file.readAll();
        file.close();

        return fileContent;

    } else {
        qWarning() << "Error in opening file " << fileUrl << ": " << file.errorString();
    }

    return QVariant();
}

QString AppSpecCPP::generateRandomPassword() {
    const QString possibleCharacters("abcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 8;

    QString randomString;
    for(int i = 0; i < randomStringLength; ++i)
    {
        int index = rand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }

    qDebug() << Q_FUNC_INFO << __LINE__ << randomString;
    return randomString;
}

QString AppSpecCPP::decodeLockPassword(QString pass)
{
    const int MOD = 10000;  // Modulo to ensure a 4-digit number
    int hashValue = 0;
    int prime = 31;  // A small prime number to generate a unique hash

    for (char c : pass.toStdString()) {
        // Calculate the contribution of each character (a=1, b=2, ..., z=26)
        int charValue = c - '0' + 1;
        hashValue = (hashValue * prime + charValue) % MOD;
    }

    std::ostringstream ss;
    ss << std::setw(4) << std::setfill('0') << hashValue;


    qDebug() << Q_FUNC_INFO << __LINE__ << QString::fromStdString(ss.str());
    return QString::fromStdString(ss.str());
}

QString AppSpecCPP::systemModeToString(SystemMode systemMode) {
    switch(systemMode) {
    case AppSpecCPP::Heating:
        return "Heating";
    case AppSpecCPP::Cooling:
        return "Cooling";
    case AppSpecCPP::Vacation:
        return "Vacation";
    case AppSpecCPP::Auto:
        return "Auto";

    case AppSpecCPP::Off:
    case AppSpecCPP::Emergency:
        break;
    }

    return "Off";
}

QString AppSpecCPP::apiBaseServerUrl() {
    return API_SERVER_BASE_URL;
}

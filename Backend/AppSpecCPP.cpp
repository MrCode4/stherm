#include "AppSpecCPP.h"
#include <QQmlEngine>

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
    }

    return Conventional;
}

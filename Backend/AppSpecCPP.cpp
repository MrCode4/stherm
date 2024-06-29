#include "AppSpecCPP.h"
#include <QQmlEngine>
#include <QFile>

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
        return file.readAll();

    } else {
        qWarning() << "Error in opening file " << fileUrl << ": " << file.errorString();
    }

    return QVariant();
}

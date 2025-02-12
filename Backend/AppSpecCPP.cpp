#include "AppSpecCPP.h"

#include <QQmlEngine>
#include <QFile>
#include <sstream>

#include "device_config.h"

static QMap<AppSpecCPP::AlertTypes, QString> AlertToMessageMap() {
    static QMap<AppSpecCPP::AlertTypes, QString> alertStringMap;

    if (alertStringMap.isEmpty()) {
        alertStringMap[AppSpecCPP::Alert_temp_high] = QString("Temperature is very high.");
        alertStringMap[AppSpecCPP::Alert_temp_low]  = QString("Temperature is very low.");
        alertStringMap[AppSpecCPP::Alert_humidity_high] = QString("Humidity is very high.");
        alertStringMap[AppSpecCPP::Alert_humidity_low]  = QString("Humidity is very low.");
        alertStringMap[AppSpecCPP::Alert_temperature_humidity_malfunction] = QString("Temperature and Humidity sensor malfunction.  Please contact your contractor.");
        alertStringMap[AppSpecCPP::Alert_Tvoc_high] = QString("Tvoc is high");
        alertStringMap[AppSpecCPP::Alert_etoh_high] = QString("etoh is high");

        alertStringMap[AppSpecCPP::Alert_iaq_high] = QString("Air Quality Sensor Malfunction\nPlease contact your contractor.");
        alertStringMap[AppSpecCPP::Alert_iaq_low]  = alertStringMap[AppSpecCPP::Alert_iaq_high];
        alertStringMap[AppSpecCPP::Alert_c02_high] = alertStringMap[AppSpecCPP::Alert_iaq_high];
        alertStringMap[AppSpecCPP::Alert_c02_low]  = alertStringMap[AppSpecCPP::Alert_iaq_high];

        alertStringMap[AppSpecCPP::Alert_fan_High] = QString("Fan Malfunction\nPlease contact your contractor.");
        alertStringMap[AppSpecCPP::Alert_pressure_high] = QString("Pressure is high");
        alertStringMap[AppSpecCPP::Alert_wiring_not_connected] = QString("Wiring is not connected.");
        alertStringMap[AppSpecCPP::Alert_could_not_set_relay] = QString("Could not set relay.");

        alertStringMap[AppSpecCPP::Alert_Light_High] = QString("Ambient sensor malfunction.\nPlease contact your contractor.");
        alertStringMap[AppSpecCPP::Alert_Light_Low]  = alertStringMap[AppSpecCPP::Alert_Light_High];

        alertStringMap[AppSpecCPP::Alert_Efficiency_Issue] =  QString("**System Efficiency Issue:**\nThe system is unable to reach the set temperature.");
        alertStringMap[AppSpecCPP::Alert_No_Data_Received] =  QString("Controller failure.\nPlease contact your contractor.");
        alertStringMap[AppSpecCPP::Alert_Air_Quality] =  QString("Poor air quality detected. Please ventilate the room.");
        alertStringMap[AppSpecCPP::Alert_Too_Long_Aux] =  QString("Auxiliary heating is running non stop for 1 hour, if this is normal for your HVAC system ignore the alert, otherwise please contact your Contractor.");
    }

    return alertStringMap;
};

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

double AppSpecCPP::defaultEmergencyTemperatureDifferenceF() {
    return 2.9;
}

double AppSpecCPP::defaultEmergencyTemperatureDifferenceC() {
    return 1.6;
}

int AppSpecCPP::defaultEmergencyMinimumTime() {
    return 2;
}

double AppSpecCPP::defaultHumidity()
{
    return 45.0;
}

QString AppSpecCPP::systemTypeString(SystemType systemType, bool camelCase) {
    switch (systemType) {
    case Conventional:
        return camelCase ? "Traditional" : "traditional";

    case HeatPump:
        return camelCase ? "HeatPump" : "heat_pump";

    case CoolingOnly:
        return camelCase ? "Cooling" : "cooling";

    case HeatingOnly:
        return camelCase ? "Heating" : "heating";

    case DualFuelHeating:
        return camelCase ? "DualFuelHeating" : "dual_fuel_heating";

    default:
        break;
    }

    return camelCase ? "Traditional" : "traditional";
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

QString AppSpecCPP::alertTypeToMessage(const AlertTypes &alertType)
{
    auto alertToMessageMap = AlertToMessageMap();
    if (alertToMessageMap.contains(alertType)) {
        return alertToMessageMap.value(alertType);
    }

     return "Unknown";
}

AppSpecCPP::AlertTypes AppSpecCPP::messageToAlertType(const QString &message)
{
    auto alertToMessageMap = AlertToMessageMap();
    if (alertToMessageMap.values().contains(message)) {
        return alertToMessageMap.key(message);
    }

    return AlertTypes::NO_ALlert;
}

QString AppSpecCPP::alertTypeToString(const AlertTypes &alertType)
{
    switch (alertType) {
    case AlertTypes::Alert_temp_high:
        return "temp_high";
    case AlertTypes::Alert_temp_low:
        return "temp_low";
    case AlertTypes::Alert_Tvoc_high:
        return "tvoc_high";
    case AlertTypes::Alert_etoh_high:
        return "etoh_high";

    case AlertTypes::Alert_iaq_high:
    case AlertTypes::Alert_iaq_low:
    case AlertTypes::Alert_c02_high:
    case AlertTypes::Alert_c02_low:
        return "air_quality_malfunction";

    case AlertTypes::Alert_humidity_high:
        return "humidity_high";
    case AlertTypes::Alert_humidity_low:
        return "humidity_low";
    case AlertTypes::Alert_pressure_high:
        return "pressure_high";

    case AlertTypes::Alert_fan_High:
    case AlertTypes::Alert_fan_low:
        return "fan_malfunction";

    case AlertTypes::Alert_wiring_not_connected:
        return "wiring_not_connected";
    case AlertTypes::Alert_temperature_not_reach:
        return "temperature_not_reach";
    case AlertTypes::Alert_temperature_humidity_malfunction:
        return "temperature_humidity_malfunction";
    case AlertTypes::Alert_Light_High:
        return "light_High";
    case AlertTypes::Alert_Light_Low:
        return "light_Low";
    case AlertTypes::Alert_Efficiency_Issue:
        return "efficiency_issue";
    case AlertTypes::Alert_No_Data_Received:
        return "no_data_received";
    case AlertTypes::Alert_Air_Quality:
        return "air_quality";
    case AlertTypes::Alert_Too_Long_Aux:
        return "too_long_aux";

    default:
        return "Unknown";
    }
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

QString AppSpecCPP::systemModeToString(SystemMode systemMode, bool camelCase) {
    switch(systemMode) {
    case AppSpecCPP::Heating:
        return camelCase ? "Heating" : "heating";
    case AppSpecCPP::Cooling:
        return camelCase ? "Cooling" : "cooling";
    case AppSpecCPP::Vacation:
        return camelCase ? "Vacation" : "vacation";
    case AppSpecCPP::Auto:
        return camelCase ? "Auto" : "auto";

    case AppSpecCPP::EmergencyHeat:
        return camelCase ? "Emergency Heat" : "emergency_heat";

    case AppSpecCPP::Off:
    case AppSpecCPP::Emergency:
        break;
    case AppSpecCPP::SMUnknown:
        break;
    }

    return camelCase ? "Off" : "off";
}

QString AppSpecCPP::apiBaseServerUrl() {
    return API_SERVER_BASE_URL;
}

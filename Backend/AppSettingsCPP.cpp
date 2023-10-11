#include "AppSettingsCPP.h"

AppSettingsCPP::AppSettingsCPP(QObject *parent)
    : QObject{parent}
{
    this->mInstance = this;   // this class is intialized in qml
}

AppSettingsCPP *AppSettingsCPP::mInstance = nullptr;
AppSettingsCPP *AppSettingsCPP::instance()
{
    return mInstance;
}

void AppSettingsCPP::setValue(const QString &propName, QVariant value)
{
    mSettings.setValue(propName, value);
}

QVariant AppSettingsCPP::value(const QString &propName)
{
    return mSettings.value(propName);
}

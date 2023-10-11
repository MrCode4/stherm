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

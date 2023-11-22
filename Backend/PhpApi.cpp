#include "PhpApi.h"

PhpAPI::PhpAPI(QObject *parent)
    : QObject{parent}
    , m_hardware(new php_hardware(m_deviceConfig, m_timing, m_currentStage, m_sensors, this))
{
    _uid = UtilityHelper::getCPUInfo().toStdString();
}

int PhpAPI::getStartMode()
{
    return m_hardware->getStartMode(_uid);
}

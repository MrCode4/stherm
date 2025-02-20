#include "PhpApi.h"

PhpAPI *PhpAPI::m_instance = nullptr;

PhpAPI *PhpAPI::instance() {
    if (!m_instance) {
        m_instance = new PhpAPI();
    }
    return m_instance;
}


PhpAPI::PhpAPI(QObject *parent)
    : QObject{parent},
    m_timing(new Timing(this))
    , m_hardware(new php_hardware(m_deviceConfig, *m_timing, m_currentStage, m_sensors, this))
{
    _uid = UtilityHelper::getCPUInfo().toStdString();

}

int PhpAPI::getStartMode()
{
    return m_hardware->getStartMode(_uid);
}

Timing* PhpAPI::timing() {
    return m_timing;
}

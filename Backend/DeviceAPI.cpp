#include "DeviceAPI.h"

#include "UtilityHelper.h"

#include "Core/System.h"
#include "Core/hardware.h"

DeviceAPI::DeviceAPI(QObject *parent)
    : QObject{parent}
    , m_system(new NUVE::System(this))
    , m_hardware(
          new NUVE::Hardware(m_deviceConfig, m_timing, m_currentStage, m_sensors, *m_system, this))
{
    _uid = UtilityHelper::getCPUInfo().toStdString();
}

int DeviceAPI::getStartMode()
{
    return m_hardware->getStartMode(_uid);
}

NUVE::Timing* DeviceAPI::timing() {
    return &m_timing;
}

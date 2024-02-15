#include "DeviceAPI.h"

#include "LogHelper.h"
#include "UtilityHelper.h"

#include "Core/Sync.h"
#include "Core/System.h"
#include "Core/hardware.h"

DeviceAPI::DeviceAPI(QObject *parent)
    : QObject{parent}
    , m_sync(new NUVE::Sync(this))
    , m_system(new NUVE::System(this))
    , m_hardware(
          new NUVE::Hardware(m_deviceConfig, m_timing, m_currentStage, m_sensors, *m_system, this))
{
    _uid = UtilityHelper::getCPUInfo().toStdString();

    m_system->setUID(_uid);
}

int DeviceAPI::runDevice()
{
    return m_hardware->runDevice(_uid);
}

int DeviceAPI::checkSN()
{
    // Check serial number
    if (m_deviceConfig.serial_number != "") {
        // serial number already set, starting normally
        // TODO what do these return values mean?
        // send is ready?
        return 1;
    }

    auto sn = m_system->getSN(_uid);
    if (sn.empty()) {

        qWarning() << "serial number empty: ";

        // Staring first time setup
        m_hardware->setDefaultValues(_uid);
        m_deviceConfig.start_mode = 2;
        return 2;
    }

    TRACE << "serial number : " << QString::fromStdString(sn);

    // staring normally, but defaults not set
    m_timing.setDefaultValues();
    m_currentStage.timestamp = NUVE::current_timestamp();
    m_deviceConfig.serial_number = sn;
    m_deviceConfig.start_mode = 1;
    return 1;
}

int DeviceAPI::getStartMode()
{
    return m_hardware->getStartMode();
}

NUVE::Timing* DeviceAPI::timing() {
    return &m_timing;
}

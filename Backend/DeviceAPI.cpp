#include "DeviceAPI.h"

#include "LogHelper.h"
#include "UtilityHelper.h"

#include "Core/Sync.h"
#include "Core/System.h"
#include "Core/hardware.h"

DeviceAPI::DeviceAPI(QObject *parent)
    : QObject{parent}
    , m_sync(new NUVE::Sync(this))
    , m_system(new NUVE::System(m_sync, this))
    , m_hardware(
          new NUVE::Hardware(m_deviceConfig, m_timing, m_currentStage, m_sensors, *m_system, this))
{
#ifdef __unix__
    _uid = UtilityHelper::getCPUInfo().toStdString();

#else
    // Use in test
    _uid = m_deviceConfig.uid;
#endif

    m_system->setUID(_uid);
    m_system->setControlAlertEnabled(m_deviceConfig.controlAlertEnabled);
}

QString DeviceAPI::uid() const
{
    return QString::fromStdString(_uid);
}

NUVE::Timing* DeviceAPI::timing()
{
    return &m_timing;
}

NUVE::System* DeviceAPI::system()
{
    return m_system;
}

NUVE::Sync* DeviceAPI::sync()
{
    return m_sync;
}

const NUVE::DeviceConfig &DeviceAPI::deviceConfig() const
{
    return m_deviceConfig;
}

void DeviceAPI::setSampleRate(const int sampleRate)
{
    if (sampleRate < 0) {
        return;
    }

    m_deviceConfig.setSampleRate(sampleRate);
}

void DeviceAPI::setEndpoint(const QString &endpoint)
{
    m_deviceConfig.endpoint = endpoint.toStdString();
    m_deviceConfig.save();
}

int DeviceAPI::getStartMode()
{
    return m_hardware->getStartMode();
}

int DeviceAPI::runDevice()
{
    return m_hardware->runDevice(_uid);
}

int DeviceAPI::checkSN()
{
    // Check serial number
    // serial number already set, starting normally
    if (m_deviceConfig.serial_number != "") {
        m_system->setSerialNumber(QString::fromStdString(m_deviceConfig.serial_number));
        m_deviceConfig.start_mode = 0;
        return 0;
    }

    // can take some time in the initial usage, but not blocking ui as the WiFi is not connected for sure
    m_system->fetchSerialNumber(uid());
    auto serialNumber = m_system->serialNumber();
    if (!m_system->hasClient()) {

        qWarning() << "serial number(SN) with false has_client, SN: " << serialNumber;

        // Staring first time setup
        m_hardware->setDefaultValues(_uid);
        m_deviceConfig.start_mode = 2;
        return 2;
    }

    TRACE << "serial number : " << serialNumber;

    // staring normally, but defaults not set
    m_timing.setDefaultValues();
    m_currentStage.timestamp = NUVE::current_timestamp();
    m_deviceConfig.serial_number = serialNumber.toStdString();
    m_deviceConfig.start_mode = 1;
    m_deviceConfig.save();
    return 1;
}

void DeviceAPI::forgetDevice()
{
    m_deviceConfig.initialise("");
}


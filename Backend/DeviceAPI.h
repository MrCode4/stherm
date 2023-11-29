#pragma once

#include <QObject>
#include <QQmlEngine>

#include "Device/current_stage.h"
#include "Device/device_config.h"
#include "Device/sensors.h"
#include "Device/timing.h"

/* PHP, on V1, is used to extend the JS web browser based UI.
 * we intend to replace it with something more suitable
 *
*/
namespace NUVE {
class System;
class Hardware;
} // namespace NUVE
class DeviceAPI : public QObject
{
    Q_OBJECT
    /* TODO - FYR
     * The Q_PROPERTY macro declares a property that could be accessed from QML.
     * e.g.
     *  Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    */
    QML_ELEMENT
public:
    explicit DeviceAPI(QObject *parent = nullptr);

    Q_INVOKABLE int getStartMode();

    NUVE::Timing *timing();

signals:

private:
    NUVE::DeviceConfig m_deviceConfig;
    NUVE::Timing m_timing;
    NUVE::CurrentStage m_currentStage;
    NUVE::Sensors m_sensors;

    NUVE::System *m_system;
    NUVE::Hardware *m_hardware;

    NUVE::cpuid_t _uid;
};

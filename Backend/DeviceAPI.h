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
class Sync;
class System;
class Hardware;
} // namespace NUVE
class DeviceAPI : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString uid READ uid NOTIFY uidChanged)

    QML_ELEMENT
public:
    explicit DeviceAPI(QObject *parent = nullptr);

    QString uid() const;
    NUVE::Timing *timing();
    NUVE::System* system();
    NUVE::Sync* sync();
    const NUVE::DeviceConfig& deviceConfig() const;
    void setSampleRate(const int sampleRate);
    void setEndpoint(const QString &endpoint);

    Q_INVOKABLE int getStartMode();
    int runDevice();
    int checkSN();

    //! Forget device configs
    void forgetDevice();

    Q_INVOKABLE void setTestConfigs(const QString &ip, const QString &user,
                                    const QString &pass, const QString &destination);

    Q_INVOKABLE QVariantMap testConfigs() const;

signals:
    void uidChanged();

private:
    NUVE::DeviceConfig m_deviceConfig;
    NUVE::Timing m_timing;
    NUVE::CurrentStage m_currentStage;
    NUVE::Sensors m_sensors;

    NUVE::Sync *m_sync;
    NUVE::System *m_system;
    NUVE::Hardware *m_hardware;

    NUVE::cpuid_t _uid;
};

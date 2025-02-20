#pragma once

#include <QObject>
#include <QQmlEngine>
#include "include/currentstage.h"
#include "include/deviceconfig.h"
#include "include/sensors.h"
#include "include/timing.h"
#include "php_hardware.h"

/* PHP, on V1, is used to extend the JS web browser based UI.
 * we intend to replace it with something more suitable
 *
*/

class PhpAPI : public QObject
{
    Q_OBJECT
    /* TODO - FYR
     * Use a wrapper for QML
     * The Q_PROPERTY macro declares a property that could be accessed from QML.
     * e.g.
     *  Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    */
    QML_ELEMENT
public:

    static PhpAPI* instance();

    Q_INVOKABLE int getStartMode();

    Timing *timing();

signals:

private:
    explicit PhpAPI(QObject *parent = nullptr);

private:
    static PhpAPI *m_instance;

    DeviceConfig m_deviceConfig;
    Timing* m_timing;
    CurrentStage m_currentStage;
    Sensors m_sensors;

    php_hardware *m_hardware;

    cpuid_t _uid;
};

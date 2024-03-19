#pragma once

#include <QObject>
#include <QQuickItem>

/*! ***********************************************************************************************
 * Base class for application specifications
 * All enums, consts and app specs should be defined here
 * A qml extension is also defined (AppSpec.qml)
 * ************************************************************************************************/

// TODO : Add control modes as Appspec properties

class AppSpecCPP : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit AppSpecCPP(QObject *parent = nullptr);
    static AppSpecCPP *instance();

    // Useage in QML: ex. AppSpecCPP.Cooling
    enum SystemMode {
        Cooling = 0,
        Heating,
        Auto,
        Vacation,
        Off,
        Emergency
    };
    Q_ENUM(SystemMode)

    enum SystemType
    {
        Conventional = 0,
        HeatPump,
        CoolingOnly,
        HeatingOnly,
        SysTUnknown
    };
    Q_ENUM(SystemType)

    enum AccessoriesType
    {
        Humidifier = 0,
        Dehumidifier,
    };
    Q_ENUM(AccessoriesType)

    enum AccessoriesWireType
    {
        T1PWRD = 0,
        T1Short,
        T2PWRD,
        None
    };
    Q_ENUM(AccessoriesWireType)

    enum FanMode {
        FMAuto = 0,
        FMOn,
        FMOff
    };
    Q_ENUM(FanMode)

    enum NightMode {
        NMOn = 0,
        NMOff
    };
    Q_ENUM(NightMode)

    //! Schedule Type
    enum ScheduleType {
        Away   = 0,
        Night  = 1,
        Home   = 2,
        Custom = 3,
        STUnknown
    };
    Q_ENUM(ScheduleType)

signals:

private:
    //! Singleton instance
    static AppSpecCPP *mInstance;

};

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


    Q_INVOKABLE QString systemTypeString(SystemType systemType);
    Q_INVOKABLE SystemType systemTypeToEnum(QString systemTypeStr);

    enum AccessoriesType
    {
        Humidifier = 0,
        Dehumidifier,
        ATNone
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
    Q_INVOKABLE QString accessoriesWireTypeString(AccessoriesWireType wt);
    Q_INVOKABLE AccessoriesWireType accessoriesWireTypeToEnum(QString wtStr);


    enum FanMode {
        FMAuto = 0,
        FMOn,
        FMOff
    };
    Q_ENUM(FanMode)

    //! Schedule Type
    enum ScheduleType {
        Away   = 0,
        Night  = 1,
        Home   = 2,
        Custom = 3,
        STUnknown
    };
    Q_ENUM(ScheduleType)

    //! Edit Mode
    enum EditMode {
        EMSchedule           = 0,
        EMHold               = 1,
        EMFan                = 2,
        EMVacation           = 3,
        EMRequestedHumidity  = 4,
        EMDesiredTemperature = 5,
        EMSensors            = 6,
        EMSettings           = 7,
        EMBacklight          = 8,
        EMSystemSetup        = 9,
        EMSystemMode         = 10,

        EMNone
    };
    Q_ENUM(EditMode)

signals:

private:
    //! Singleton instance
    static AppSpecCPP *mInstance;

};

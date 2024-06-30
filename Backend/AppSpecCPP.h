#pragma once

#include <QObject>
#include <QQuickItem>

/*! ***********************************************************************************************
 * Base class for application specifications
 * All enums, consts and app specs should be defined here
 * A qml extension is also defined (AppSpec.qml)
 * ************************************************************************************************/
/* ************************************************************************************************
 * Main data keys
 * ************************************************************************************************/
const QString temperatreKey = QString("temperature");
const QString humidityKey   = QString("humidity");
const QString co2Key        = QString("co2");
const QString etohKey       = QString("etoh");
const QString TvocKey       = QString("Tvoc");
const QString iaqKey        = QString("iaq");
const QString pressureKey   = QString("pressure");
const QString RangeMilliMeterKey   = QString("RangeMilliMeter");
const QString brightnessKey = QString("brightness");
const QString fanSpeedKey   = QString("fanSpeed");

// TODO : Add control modes as Appspec properties

class AppSpecCPP : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit AppSpecCPP(QObject *parent = nullptr);
    static AppSpecCPP *instance();

    enum ChangeType {
        ctNone               = 0,
        ctCurrentTemperature = 1 << 0,
        ctSetTemperature     = 1 << 1,
        ctMode               = 1 << 2,
        ctCurrentHumidity    = 1 << 3,
        ctSetHumidity        = 1 << 4,
        ctSendRelay          = 1 << 5,
        ctDefault            = ctSetTemperature | ctMode | ctCurrentHumidity,
        ctAll                = ctDefault | ctCurrentTemperature | ctCurrentHumidity,
    };
    Q_ENUM(ChangeType)
    Q_DECLARE_FLAGS(ChangeTypes, ChangeType);

    enum CPUGovernerOption {
        CPUGpowersave = 0,
        CPUGondemand,
        CPUGperformance,
        CPUGUnknown
    };
    Q_ENUM(CPUGovernerOption)


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

    //! Edit Mode
    enum EditMode {
        EMNone               = 0,
        EMSchedule           = 1 << 0,
        EMHold               = 1 << 1,
        EMFan                = 1 << 2,
        EMVacation           = 1 << 3,
        EMRequestedHumidity  = 1 << 4,
        EMDesiredTemperature = 1 << 5,
        EMSensors            = 1 << 6,
        EMSettings           = 1 << 7,
        EMBacklight          = 1 << 8,
        EMSystemSetup        = 1 << 9,
        EMSystemMode         = 1 << 10,
        EMDateTime           = 1 << 11,
    };
    Q_ENUM(EditMode)


    /**
 * @brief Enumeration for alert types.
 */
    enum AlertTypes
    {
        Alert_temp_high = 1,// +127 max
        Alert_temp_low, // -128 low
        Alert_Tvoc_high, // 255 max (tvoc value range 0.1 to 10+ mg/m^3 value is divided by 10.0)
        Alert_etoh_high, //up to 20ppm
        Alert_iaq_high, //1 to 5
        Alert_iaq_low, //1 to 5
        Alert_humidity_high,// up to 100%
        Alert_humidity_low,//as low as 0%
        Alert_pressure_high, //up to 1200 hPa
        Alert_c02_high,//400 to 5000ppm
        Alert_c02_low,//400 to 5000ppm
        Alert_fan_High,// 4200 RPM
        Alert_fan_low,// 3800 RPM
        Alert_wiring_not_connected,
        Alert_could_not_set_relay,
        Alert_temperature_not_reach,
        Alert_Light_High,
        Alert_Light_Low,
        NO_ALlert
    };
    Q_ENUM(AlertTypes)


    Q_INVOKABLE QVariant readFromFile(const QString &fileUrl);

signals:

private:
    //! Singleton instance
    static AppSpecCPP *mInstance;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(AppSpecCPP::ChangeTypes)

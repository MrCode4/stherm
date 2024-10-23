pragma Singleton

import QtQuick

import QtQuickStream
import Stherm

AppSpecCPP {

    property int readInterval: 1000
    property int minStepTempC: 6
    property int minStepTempF: 10
    property int minStepHum:   20

    property int minimumFanWorking: 5
    property int maximumFanWorking: 55

    property int minimumHumidity: 20
    property int maximumHumidity: 70

    //! Default for temperature unit
    property int defaultTemperatureUnit: AppSpec.TempratureUnit.Fah

    //! Maximum value of first temperature handler (left) in auto mode
    //! Celcius
    property real maxAutoMinTemp: 29.4444   // 85 F

    //! Minimum value of second temperature handler (right) in auto mode
    //! Celcius
    property real minAutoMaxTemp: 15.5556   // 60 F

    //! Default of requested temperature
    //! Celcius
    property real defaultRequestedTemperature: 22.22 // 72 F

    //! Diffrence between autoMinTemp and autoMaxTemp
    //! Celcius
    property real autoModeDiffrenceC: 2.0
    property real autoModeDiffrenceF: 3.0

    //! Auto mode defaults
    //! Celcius
    property real defaultAutoMinReqTemp: 20.0    // 68 F
    property real defaultAutoMaxReqTemp: 23.3333 // 74 F

    //! Dual fuel temperature range
    //! Fahrenheit
    property real maximumDualFuelThresholdF: 45
    property real minimumDualFuelThresholdF: 15

    //! Percent
    property int defaultBrightness: 100
    //! Percent
    property int defaultVolume:     50


    //! To improve efficiency, we should delete any messages that exceed
    //! the maximum limit of messagesLimit messages.
    property int messagesLimits: 50

    enum TestModeType {
        SerialNumber = 0, //! Test mode started due to serial number
        StartMode,        //! Test mode started due to TI board
        User,
        None
    }

    enum DeviceType {
        DT_IMX6 = 0,
        DT_Sim,
        DT_Unknown
    }

    enum InstallationType {
        ITNewInstallation = 0,
        ITExistingSystem,
        ITUnknown
    }

    enum TempratureUnit {
        Cel,    //! Celsius
        Fah     //! Fahrenheit
    }

    enum TimeFormat {
        Hour12,
        Hour24
    }

    //! Type of a Sensor
    enum SensorType {
        OnBoard,
        Wireless
    }

    //! Sensor locations
    enum SensorLocation {
        Unknown,
        Other,
        Bedroom,
        LivingRoom,
        KidsRoom,
        Bathroom,
        Kitchen,
        Basement,
        MainFloor,
        Office,
        Upstairs,
        Downstairs,
        DinningRoom,
        GuestHouse
    }

    // Residence types
    enum ResidenceTypes {
        Residental = 0,
        Commercial,
        Unknown
    }

    //! Device location map
    property var residenceTypesNames: {
        var types = {};
        types[`${AppSpec.ResidenceTypes.Residental}`] = "Residental";
        types[`${AppSpec.ResidenceTypes.Commercial}`] = "Commercial";

        return types;
    }

    //! Device location map
    property var deviceLoacations: {
        var types = {};
        types[`${AppSpec.ResidenceTypes.Residental}`] = ["Basement", "Bedroom", "Dinning room",
                                                         "Downstairs", "Guesthouse", "Kids room",
                                                         "Living room", "Main floor", "Master bedroom",
                                                         "Office", "Upstairs", "Other"];
        types[`${AppSpec.ResidenceTypes.Commercial}`] = ["Lunchroom", "Office", "Warehouse", "Other"];

        types[`${AppSpec.ResidenceTypes.Unknown}`]    = [];

        return types;
    }

    //! Sensor location names
    //! \note: we use map (js object) instead of array to avoid bugs in case of moving
    //! SensorLocation enum values
    readonly property var sensorLocationNames: {
        var names = {};
        names[`${AppSpec.SensorLocation.Unknown}`]      = "Unknown";
        names[`${AppSpec.SensorLocation.Other}`]        = "Other";
        names[`${AppSpec.SensorLocation.Bedroom}`]      = "Bedroom";
        names[`${AppSpec.SensorLocation.LivingRoom}`]   = "Living Room";
        names[`${AppSpec.SensorLocation.KidsRoom}`]     = "Kids Room";
        names[`${AppSpec.SensorLocation.Bathroom}`]     = "Bathroom";
        names[`${AppSpec.SensorLocation.Kitchen}`]      = "Kitchen";
        names[`${AppSpec.SensorLocation.Basement}`]     = "Basement";
        names[`${AppSpec.SensorLocation.MainFloor}`]    = "Main Floor";
        names[`${AppSpec.SensorLocation.Office}`]       = "Office";
        names[`${AppSpec.SensorLocation.Upstairs}`]     = "Upstairs";
        names[`${AppSpec.SensorLocation.Downstairs}`]   = "Downstairs";
        names[`${AppSpec.SensorLocation.DinningRoom}`]  = "Dinning Room";
        names[`${AppSpec.SensorLocation.GuestHouse}`]   = "Guesthouse";

        return names
    }

    readonly property var scheduleTypeNames: {
        var names = {};

        names[`${AppSpec.Away}`]      = "Away";
        names[`${AppSpec.Night}`]     = "Night";
        names[`${AppSpec.Home}`]      = "Home";
        names[`${AppSpec.Custom}`]    = "Custom";

        return names
    }

    function scheduleNameToType(typeName: string) {
        var type = AppSpec.STUnknown;
        let index = Object.values(scheduleTypeNames).findIndex(elem => elem === typeName);

        if (index !== -1)
            type = Object.keys(scheduleTypeNames)[index];

        return type;
    }

    //! Minimum and maximum temperature in the app (Celcius)
    property real minimumTemperatureC: 18
    property real maximumTemperatureC: 30
    property real minimumTemperatureF: 65
    property real maximumTemperatureF: 85


    //! Minimum and maximum temperature in the vacation (Celcius)
    property real vacationMinimumTemperatureC: 4
    property real vacationMaximumTemperatureC: 32
    property real vacationMinimumTemperatureF: 40
    property real vacationMaximumTemperatureF: 90

    //! Minimum and maximum temperature in the vacation (Celcius)
    property real autoMinimumTemperatureC: 4
    property real autoMaximumTemperatureC: 32
    property real autoMinimumTemperatureF: 40
    property real autoMaximumTemperatureF: 90

    //! Get default schedule
    function getDefaultSchedule (type: int) : SceduleCPP {

        var newSchedule = QSSerializer.createQSObject("ScheduleCPP", ["Stherm", "QtQuickStream"]);
        newSchedule.type = type;

        switch (type) {
        case AppSpecCPP.Away: {
            newSchedule.minimumTemperature = 24.44444; // 76
            newSchedule.maximumTemperature = 26.66667; // 80
            newSchedule.startTime = "06:00 AM";
            newSchedule.endTime = "03:00 PM";
            newSchedule.repeats = "Mo,Tu,We,Th,Fr";
        } break;

        case AppSpecCPP.Night: {
            newSchedule.minimumTemperature = 23.33333; // 74
            newSchedule.maximumTemperature = 25.55556; // 78
            newSchedule.startTime = "10:00 PM";
            newSchedule.endTime = "06:00 AM";
            newSchedule.repeats = "Mo,Tu,We,Th,Fr,Sa,Su";
        } break;

        case AppSpecCPP.Home: {
            newSchedule.minimumTemperature = 22.77778; // 73
            newSchedule.maximumTemperature = 25.0; // 77
            newSchedule.startTime = "09:00 AM";
            newSchedule.endTime = "06:00 PM";
            newSchedule.repeats = "Mo,Tu,We,Th,Fr";
        } break;

        default: {

        }
        }

        return newSchedule;
    }

    //! Convert temperature unit to string
    function temperatureUnitString(unit: int) : string {

        if (unit === null || unit === undefined) {
            unit = defaultTemperatureUnit;
        }

        if (unit === AppSpec.TempratureUnit.Cel) {
            return "C";

        } else if (unit === AppSpec.TempratureUnit.Fah) {
            return "F";
        }

        // Can not happen
        return "F";

    }

    //! airQuality <= 1.0
    readonly property real airQualityGood: 1.0

    //! airQuality > 3.0
    readonly property real airQualityPoor: 3.0

    //! Air quality alert threshold
    readonly property real airQualityAlertThreshold: 10.0

    //! Icon for show notification when a software update is available
    readonly property string swUpdateIcon: "qrc:/Stherm/Images/icons8-installing-updates-50.png"

    //! Icon for show notification when an alert is available (Use in the screen saver page)
    readonly property string alertIcon: "qrc:/Stherm/Images/icons8-error-48.png"

    //! Icon for show notification when a message is available (Use in the screen saver page)
    readonly property string messageIcon: "qrc:/Stherm/Images/icons8-message-50.png"
}

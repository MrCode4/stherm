pragma Singleton

import QtQuick

import QtQuickStream
import Stherm

AppSpecCPP {

    property int readInterval: 1000
    property int minStepTempC: 6
    property int minStepTempF: 10
    property int minStepHum:   20

    //! Maximum value of first temperature handler (left) in auto mode
    //! Celcius
    property real maxAutoMinTemp: 29.4444

    //! Minimum value of second temperature handler (right) in auto mode
    //! Celcius
    property real minAutoMaxTemp: 15.5556

    //! Percent
    property int defaultBrightness: 100
    //! Percent
    property int defaultVolume:     50

    enum DeviceType {
        DT_IMX6 = 0,
        DT_Sim,
        DT_Unknown
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

    //! Get default schedule
    function getDefaultSchedule (type: int) : SceduleCPP {

        var newSchedule = QSSerializer.createQSObject("ScheduleCPP", ["Stherm", "QtQuickStream"]);
        newSchedule.type = type;

        switch (type) {
        case AppSpecCPP.Away: {
            newSchedule.temprature = 25.55;           // 78 F
            newSchedule.startTime = "06:00 AM";
            newSchedule.endTime = "03:00 PM";
            newSchedule.repeats = "Mo,Tu,We,Th,Fr";
        } break;

        case AppSpecCPP.Night: {
            newSchedule.temprature = 24.44;         // 76 F
            newSchedule.startTime = "10:00 PM";
            newSchedule.endTime = "06:00 AM";
            newSchedule.repeats = "Mo,Tu,We,Th,Fr,Sa,Su";
        } break;

        case AppSpecCPP.Home: {
            newSchedule.temprature = 23.33                 // 74 F
            newSchedule.startTime = "09:00 AM";
            newSchedule.endTime = "06:00 PM";
            newSchedule.repeats = "Mo,Tu,We,Th,Fr";
        } break;

        default: {

        }
        }

        return newSchedule;
    }

    //! airQuality <= 1.0
    readonly property real airQualityGood: 1.0

    //! airQuality > 3.0
    readonly property real airQualityPoor: 3.0
}

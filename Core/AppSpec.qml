pragma Singleton

import QtQuick
import Stherm

AppSpecCPP {

    property int readInterval: 1000

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

    //! Minimum and maximum temperature in the app (Celcius)
    property real minimumTemperatureC: 18
    property real maximumTemperatureC: 30
}

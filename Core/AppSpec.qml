pragma Singleton

import QtQuick
import Stherm

AppSpecCPP {

    property int simReadInterval: 1000

    enum FanMode {
        FMAuto = 0,
        FMOn,
        FMOff
    }

    enum DeviceType {
        DT_IMX6 = 0,
        DT_Sim,
        DT_Unknown
    }

    enum SystemMode {
        Cooling,
        Heating,
        Auto,
        Vacation,
        Off
    }

    enum TempratureUnit {
        Cel,    //! Celsius
        Fah     //! Fahrenheit
    }

    enum TimeFormat {
        Hour12,
        Hour24
    }
}

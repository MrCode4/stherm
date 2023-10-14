pragma Singleton

import QtQuick 2.15
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
}

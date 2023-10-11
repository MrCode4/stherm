pragma Singleton

import QtQml 2.11
import QtQuick 2.11
import QTemplateProject

QtObject {
    readonly property string primaryColor:          "#ff6446"
    readonly property string primaryColorDarker:    Qt.darker(primaryColor)
    readonly property string primaryDarkGrey:       "#121314"   
    readonly property string primaryOffWhite:       "#f2f2f2"   
    readonly property string backgroundGray:        "#2A2A2A"   // used for example as back color of popups
    readonly property string backgroundGrayDarker1: "#1F1F1F"   // used for example for camera settings box header
    readonly property string backgroundGrayDarker2: "#161314"   // used for example for camera settings box content
    readonly property string borderLineGray:        "#474747"
    readonly property string inputBoxGray:          "#505050"
    readonly property string primaryGreen:          "#74C667"
    readonly property string primaryRed:            "#DA6767"
    readonly property string primaryTextColor:      "white"
    readonly property string hoverColor:            baseColors.grayScale15
    readonly property string selectionColor:        baseColors.grayScale12

    // Menu Icons
    property alias menuIcons: _menuIcons
    readonly property QtObject _menuIcons: QtObject {
        id: _menuIcons
        readonly property string login:             "\uf2f6"
        readonly property string about:             "\uf05a"
        readonly property string processing:        "\uf2db"
        readonly property string calibration:       "\uf05b"
        readonly property string dataReview:        "\uf093"
        readonly property string connect:           "\uf233"
        readonly property string wired:             "\uf6ff"
        readonly property string message:           "\ue1df"
        readonly property string status:            "\ue0c7"
        readonly property string streaming:         "\uf8dd"
        readonly property string conversion:        "\uf1c8"
        readonly property string refresh:           "\uf2f1"
        readonly property string review:            "\uf07b"
        readonly property string reviewSettings:    "\ue187"
        readonly property string copy:              "\uf0c5"
        readonly property string link:              "\ue1cd"
        readonly property string unlink:            "\ue1ce"
        readonly property string exit:              "\uf08b"
        readonly property string warning:           "\uf071"
        readonly property string save:              "\ue053"
        readonly property string saveAs:            "\ue182"
        readonly property string load:              "\uf093"
    }


    // Side Menu Icons
    property alias sideMenuIcons: _sideMenuIcons
    readonly property QtObject _sideMenuIcons: QtObject {
        id: _sideMenuIcons
        readonly property string layout:             "\ue290"
        readonly property string device:             "\uf5fc"
        readonly property string visualization:      "\uf201"
    }

    // General Icons
    property alias generalIcons: _generalIcons
    readonly property QtObject _generalIcons: QtObject {
        id: _generalIcons
        readonly property string folderPlus:       "\uf65e"
        readonly property string simplePlus:       "\ue59e"
        readonly property string chevronUp:        "\uf077"
        readonly property string chevronDown:      "\uf078"
        readonly property string save:             "\uf0c7"
        readonly property string run:              "\uf04b" // (fa_play)
        readonly property string stop:             "\uf04d"
        readonly property string pause:            "\uf04c"
        readonly property string info:             "\uf05a"
        readonly property string trash:            "\uf1f8"
        readonly property string visible:          "\uf06e"
        readonly property string xmark:            "\uf057"
        readonly property string palette:          "\uf53f"
        readonly property string resetData:        "\uf1da"
    }

    // Base Colors
    property alias baseColors: _baseColors
    readonly property QtObject _baseColors: QtObject {
        id: _baseColors
        readonly property string grayScale01: "#FFFFFF"
        readonly property string grayScale02: "#F0F0F0"
        readonly property string grayScale03: "#E7E7E7"
        readonly property string grayScale04: "#E5E5E5"
        readonly property string grayScale05: "#D4D4D4"
        readonly property string grayScale06: "#CCCCCC"
        readonly property string grayScale07: "#C6C6C6"
        readonly property string grayScale08: "#BBBBBB"
        readonly property string grayScale09: "#A0A0A0"
        readonly property string grayScale10: "#808080"
        readonly property string grayScale11: "#7F7F7F"
        readonly property string grayScale12: "#606060"
        readonly property string grayScale13: "#454545"
        readonly property string grayScale14: "#3C3C3C"
        readonly property string grayScale15: "#3A3D41"
        readonly property string grayScale16: "#333333"
        readonly property string grayScale17: "#303031"
        readonly property string grayScale18: "#292929"
        readonly property string grayScale19: "#252526"
        readonly property string grayScale20: "#1E1E1E"
        readonly property string grayScale21: "#000000"
    }

    readonly property var processIcons: [
        "\uf017",       // PENDING
        "\uf251",       // PROCESSING
        "\uf058",       // DONE
        "\uf057",       // FAILED
        "\uf057"        // ABORTED
    ]

    readonly property var processColors: [
        primaryColor,       // PENDING
        primaryColor,       // PROCESSING
        primaryGreen,       // DONE
        primaryRed,         // FAILED
        primaryRed          // ABORTED
    ]

    readonly property var processNames: [
        "Pending",       // PENDING
        "Processing",    // PROCESSING
        "Done",          // DONE
        "Failed",        // FAILED
        "Aborted"        // ABORTED
    ]

    readonly property var stateColors: [
        "orange",           // UNKNOWN
        primaryColor,       // STARTING
        primaryGreen,       // RUNNING
        primaryRed,         // FAILED
    ]

    readonly property var stateIcons: [
        "\uf017",           // UNKNOWN
        "\uf251",           // STARTING
        "\uf058",           // RUNNING
        "\uf057",           // FAILED
    ]

    readonly property var gaugeNames: [
        "CircularModernGauge1",
        "CircularSpeedGauge",
        "LevelGauge"
    ]

    //! Connection Types
    readonly property var connectionTypes: [
        "UNKNOWN",
        "UART",
        "CAN",
        "SPI",
        "I2C"
    ]

    //! Connection State Names
    readonly property var connectionState: [
        "Connected!",
        "Connecting...",
        "Disconnected!",
        "Failed to connect, make sure device is connected and powered on.",
        ""
    ]

    //! Connection State Colors
    readonly property var connectionStateColors: [
        "green",
        "white",
        "orange",
        "red",
        ""
    ]

    readonly property var rampStateNames: [
        "Done",
        "InProgress",
        "Error",
        "Hold",
        "Idle",
        "UnknownState"
    ]

    readonly property var rampStateColors: [
        primaryGreen,
        primaryColor,
        primaryRed,
        primaryRed,
        baseColors.grayScale10,
        baseColors.grayScale08
    ]

    //! Graph Styles
    readonly property var plotThemes: [
        "Light",
        "BlueCerulean",
        "Dark",
        "BrownSand",
        "BlueNcs",
        "HighContrast",
        "BlueIcy",
        "Qt"
    ]

    //! Data Frequency
    readonly property var dataFrequency: [
        "NoUpdate",
        "LowFreq",
        "MediumFreq",
        "HighFreq",
        "OnReset"
    ]

    //! Plot Type
    readonly property var plotType: [
        "Line",
        "Scatter"
    ]

    //! x-range divisions
    readonly property var xRangeDivisions: [
        "1 ns / Div",
        "2 ns / Div",
        "5 ns / Div",
        "10 ns / Div",
        "20 ns / Div",
        "50 ns / Div",
        "100 ns / Div",
        "200 ns / Div",
        "500 ns / Div",
        "1 us / Div",
        "2 us / Div",
        "5 us / Div",
        "10 us / Div",
        "20 us / Div",
        "50 us / Div",
        "100 us / Div",
        "200 us / Div",
        "500 us / Div",
        "1 ms / Div",
        "2 ms / Div",
        "5 ms / Div",
        "10 ms / Div",
        "20 ms / Div",
        "50 ms / Div",
        "100 ms / Div",
        "200 ms / Div",
        "500 ms / Div",
        "1 s / Div",
        "2 s / Div",
        "5 s / Div",
        "10 s / Div",
        "20 s / Div",
        "50 s / Div",
        "100 s / Div",
        "200 s / Div",
        "500 s / Div",
    ]

    //! motor fault info
    //! \important: The indices of this array is not in match with AppSpec::motorFaults enums
    readonly property var motorFaultsNames: {
        0: "No Fault",
        1: "FOC Duration",
        2: "Over Voltage",
        4: "Under Voltage",
        8: "Over Temperature",
        16: "Over Current",
        32: "Start-up",
        64: "Speed Feedback 1",
        128: "Speed Feedback 2",
        256: "Break In",
        512: "Software Error",
        1024: "Offset Calibration"
    }


    //! Motor States Names
    readonly property var motorStateNames: [
        "IDLE",
        "IDLE_ALIGNMENT",
        "ALIGNMENT",
        "IDLE_START",
        "START",
        "START_RUN",
        "RUN",
        "ANY_STOP",
        "STOP",
        "STOP_IDLE",
        "FAULT_NOW",
        "FAULT_OVER",
        "ICLWAIT",
        "ALIGN_CHARGE_BOO",
        "ALIGN_OFFSET_CAL",
        "ALIGN_CLEAR",
        "CHARGE_BOOT_CAP",
        "OFFSET_CALIB",
        "CLEAR",
        "SWITCH_OVER",
        "WAIT_STOP_MOTOR",
        "UNDEFINED"
    ]

    //! Motor States Colors
    readonly property var motorStateColors: [
        "#F4CA16",  // "IDLE",
        "#F4CA16", // "IDLE_ALIGNMENT",
        "#F4CA16", // "ALIGNMENT",
        "#F4CA16", // "IDLE_START",
        "#50C878", // "START",
        "#50C878", // "START_RUN",
        "#50C878", // "RUN",
        "#EB4C42", // "ANY_STOP",
        "#EB4C42", // "STOP",
        "#F07427", // "STOP_IDLE",
        "#FF2A1B", // "FAULT_NOW",
        "#FF2A1B", // "FAULT_OVER",
        "#bebec1", // "ICLWAIT",
        "#bebec1", // "ALIGN_CHARGE_BOO",
        "#bebec1", // "ALIGN_OFFSET_CAL",
        "#bebec1", // "ALIGN_CLEAR",
        "#FDF5E6", // "CHARGE_BOOT_CAP",
        "#FDF5E6", // "OFFSET_CALIB",
        "#bebec1", // "CLEAR",
        "#FDF5E6", // "SWITCH_OVER",
        "#FDF5E6", // "WAIT_STOP_MOTOR",
        "#bebec1", // "UNDEFINED"
    ]

    //! Serial Ports
    readonly property var baudrates: [9600, 115200, 230400, 460800, 921600, 1843200, 3686400, 6000000]


    /* Functions
     * ****************************************************************************************/
    //! Receives a process int and returns the standard color for it
    function getProcessColor(mode: int)
    {
        return processColors[mode];
    }

    //! Receives a process mode and returns the standard icon for it
    function getProcessIcon(mode: int)
    {
        return processIcons[mode];
    }

    //! Receives a process mode and returns the standard name for it
    function getProcessName(mode: int)
    {
        return processNames[mode];
    }
}

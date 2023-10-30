import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * DesiredTempratureItem provides a ui for setting desired temprature
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to I_Device
    property I_Device   device

    property  UiPreferences uiPreference

    //! Unit of temprature
    property string     unit: (uiPreference?.tempratureUnit === UiPreferences.TempratureUnit.Fah ? "F" : "C") ?? "F"

    //! Minimum temprature
    property int        minTemprature: 18

    //! Maximum temprature
    property int        maxTemprature: 30

    //! Offset of desired temp label
    property int        labelVerticalOffset: -8

    //! Holds whether SemiCircleSlider is being dragged
    readonly property alias dragging: _tempSlider.pressed

    //!
    property bool       labelVisible: true

    /* Object properties
     * ****************************************************************************************/
    //    Material.theme: Material.Dark
    implicitWidth: 360
    implicitHeight: 180
    font.pointSize: Qt.application.font.pointSize * 2.8
    background: null
    contentItem: Item {
        SemiCircleSlider {
            id: _tempSlider
            anchors.fill: parent
            enabled: labelVisible
            from: minTemprature
            to: maxTemprature
            value: device?.requestedTemp ?? 18.0

            onPressedChanged: {
                if (!pressed && device && device.requestedTemp !== value) {
                    device.requestedTemp = value;
                }
            }
        }

        //! Desired temprature label
        Label {
            id: _desiredTempratureLbl
            visible: labelVisible
            anchors.centerIn: parent
            anchors.verticalCenterOffset: labelVerticalOffset
            text: Number(uiPreference?.convertedTemperature(_tempSlider.value) ?? 0).toLocaleString(locale, "f", 0)

            //! Unit
            Label {
                anchors.left: parent.right
                anchors.top: parent.top
                anchors.topMargin: 20
                opacity: 0.6
                font {
                    pointSize: Qt.application.font.pointSize * 1.2
                    capitalization: "AllUppercase"
                }
                text: `\u00b0${unit}`
            }
        }
    }
}

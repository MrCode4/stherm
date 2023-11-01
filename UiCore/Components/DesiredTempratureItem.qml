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

    property  UiSession             uiSession

    //! Reference to I_Device
    property I_Device           device: uiSession.appModel

    //! Unit of temprature
    property string             unit: (device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C") ?? "F"

    //! Minimum temprature
    property int                minTemprature: 18

    //! Maximum temprature
    property int                maxTemprature: 30

    //! Offset of desired temp label
    property int                labelVerticalOffset: -8

    //! Holds whether SemiCircleSlider is being dragged
    readonly property alias     dragging: _tempSlider.pressed

    //! Visibility of temprature label
    property bool               labelVisible: true

    //! Label width
    readonly property alias     labelWidth: _desiredTempratureLbl.width

    /* Object properties
     * ****************************************************************************************/
    font.pointSize: Qt.application.font.pointSize * 2.8
    background: null
    contentItem: Item {
        SemiCircleSlider {
            id: _tempSlider
            anchors.centerIn: parent
            width: parent.width
            height: width / 2
            enabled: labelVisible
            from: minTemprature
            to: maxTemprature
            value: device?.requestedTemp ?? 18.0

            onPressedChanged: {
                if (!pressed && device && device.requestedTemp !== value) {
                    uiSession.deviceController.setDesiredTemperature(value);
                }
            }
        }

        //! Desired temprature label
        Label {
            id: _desiredTempratureLbl
            visible: labelVisible
            anchors.centerIn: parent
            anchors.verticalCenterOffset: labelVerticalOffset
            text: Number(Utils.convertedTemperature(_tempSlider.value, device.setting.tempratureUnit))
                  .toLocaleString(locale, "f", 0)

            //! Unit
            Label {
                anchors.left: parent.right
                anchors.top: parent.top
                opacity: 0.6
                font {
                    pointSize: _root.font.pointSize / 2
                    capitalization: "AllUppercase"
                }
                text: `\u00b0${unit}`
            }
        }

        //! Connections to sync slider with device.
        Connections {
            target: device

            //! Update slider value (UI) with changed requestedTemp
            //! When setDesiredTemperature failed, update slider with previous value.
            function onRequestedTempChanged() {
                _tempSlider.value = device.requestedTemp;
            }
        }
    }
}

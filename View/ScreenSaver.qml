import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScreenSaver Item
 * ***********************************************************************************************/
Popup {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to I_DeviceController
    property I_DeviceController     deviceController

    //! Reference to I_Device
    property I_Device               device: deviceController?.device ?? null

    //! Unit
    property string     unit: device?.setting?.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C" ?? "F"

    /* Object properties
     * ****************************************************************************************/
    implicitHeight: AppStyle.size
    implicitWidth: AppStyle.size
    closePolicy: Popup.NoAutoClose
    modal: true
    dim: false
    background: Rectangle {
        color: Style.background
    }

    /* Children
     * ****************************************************************************************/
    //! This MouseArea is used to hide ScreenSaver
    MouseArea {
        anchors.fill: parent

        onPressed: {
            ScreenSaverManager.setActive();
        }
    }

    ColumnLayout {
        id: _contentLay
        anchors.centerIn: parent

        //! Temprature Label
        Row {
            Layout.alignment: Qt.AlignCenter

            Label {
                id: _tempratureLbl

                font.pointSize: AppStyle.size / 6
                text: Number(Utils.convertedTemperature(
                                 device?.currentTemp ?? 0,
                                 device?.setting?.tempratureUnit)).toLocaleString(locale, "f", 0)

            }

            Label {
                opacity: 0.6
                font {
                    pointSize: Qt.application.font.pointSize * 2.4
                    capitalization: "AllUppercase"
                }
                text: `\u00b0${unit}`
            }
        }

        //! Mode button
        SystemModeButton {
            Layout.leftMargin: 2 * _icon.width / 3 - width / 2
            deviceController: _root.deviceController
        }

        //! NEXGEN icon
        NexgenIcon {
            id: _icon
            Layout.preferredWidth: _root.width * 0.75
            Layout.preferredHeight: sourceSize.height * width / sourceSize.width
        }
    }
}

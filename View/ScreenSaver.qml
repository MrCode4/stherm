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
            Layout.topMargin: -height / 2.5
            Layout.leftMargin: 2 * _icon.width / 3 - width
            background: null
            deviceController: _root.deviceController

            TapHandler { } //! To ensure no button functionality
        }

        //! Current time
        DateTimeLabel {
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 24
            font.pointSize: Application.font.pointSize * 1.5
            is12Hour: device?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
            showDate: false
        }

        //! NEXGEN icon
        NexgenIcon {
            id: _icon
            Layout.preferredWidth: _root.width * 0.75
            Layout.preferredHeight: sourceSize.height * width / sourceSize.width
        }
    }
}

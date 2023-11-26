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

        ColumnLayout {
            Layout.rightMargin: 48
            Layout.leftMargin: Layout.rightMargin

            //! Temprature Label
            Row {
                id: tempratureLabel
                Layout.alignment: Qt.AlignCenter

                Label {
                    id: _tempratureLbl

                    font.pointSize: Application.font.pointSize * 6.5
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
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: -16
                background: null
                deviceController: _root.deviceController

                TapHandler { } //! To ensure no button functionality
            }
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
        OrganizationIcon {
            id: _icon
            Layout.fillWidth: true
            Layout.preferredHeight: _root.height / 5
            Layout.preferredWidth: 0
        }
    }
}

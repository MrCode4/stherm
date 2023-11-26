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
        width: parent.width * 0.6

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            //! Temprature Label
            Label {
                id: _tempratureLbl

                Layout.fillWidth: true
                font.pointSize: Application.font.pointSize * 6.5
                minimumPointSize: Application.font.pointSize
                fontSizeMode: "HorizontalFit"
                horizontalAlignment: "AlignHCenter"
                text: Number(Utils.convertedTemperature(
                                 device?.currentTemp ?? 0,
                                 device?.setting?.tempratureUnit)).toLocaleString(locale, "f", 0)

                Label {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        horizontalCenterOffset: (_tempratureLbl.contentWidth + width) / 2 + 6
                    }
                    y: height / 2 + 6
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
                Layout.alignment: Qt.AlignCenter
                Layout.leftMargin: dateTimeLbl.width + width / 2
                background: null
                deviceController: _root.deviceController

                TapHandler { } //! To ensure no button functionality
            }
        }

        //! Current time
        DateTimeLabel {
            id: dateTimeLbl
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 24
            font.pointSize: Application.font.pointSize * 1.8
            is12Hour: device?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
            showDate: false
        }

        //! NEXGEN icon
        OrganizationIcon {
            id: _icon
            Layout.alignment: Qt.AlignCenter
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.fillWidth: true
        }
    }
}

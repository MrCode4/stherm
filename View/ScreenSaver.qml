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

    property bool isActiveSchedule: deviceController?.currentSchedule ?? null

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
        width: AppStyle.size

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter

            //! Temprature Label
            Label {
                id: _tempratureLbl

                Layout.alignment: Qt.AlignCenter
                Layout.rightMargin: unitLbl.width / 2
                padding: 0
                font.pointSize: Application.font.pointSize * 6.5
                minimumPointSize: Application.font.pointSize
                fontSizeMode: "HorizontalFit"
                verticalAlignment: "AlignVCenter"
                horizontalAlignment: "AlignHCenter"
                text: Number(Utils.convertedTemperature(
                                 device?.currentTemp ?? 0,
                                 device?.setting?.tempratureUnit)).toLocaleString(locale, "f", 0)

                Label {
                    id: unitLbl
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

            RowLayout {
                Layout.leftMargin: dateTimeLbl.width / 1.3
                Layout.rightMargin: dateTimeLbl.width / 2
                Layout.topMargin: -height / 2

                //! Schedule button
                RoniaTextIcon {
                    font.pointSize: Style.fontIconSize.normalPt
                    color: _root.Material.foreground
                    text: "\uf073"
                    visible: _root.isActiveSchedule
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                //! Mode button
                SystemModeButton {
                    background: null
                    deviceController: _root.deviceController

                    TapHandler { } //! To ensure no button functionality
                }
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
            Layout.leftMargin: AppStyle.size / 10
            Layout.rightMargin: AppStyle.size / 10
            Layout.fillWidth: true
            Layout.preferredHeight: _root.height / 8
        }
    }
}

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
        }

        //! Spacer
        Item {
            height: 120
            Layout.fillWidth: true
        }

        //! NEXGEN icon
        OrganizationIcon {
            id: _icon

            appModel: _root.device
            Layout.alignment: Qt.AlignCenter
            Layout.leftMargin: AppStyle.size / 10
            Layout.rightMargin: AppStyle.size / 10
            Layout.fillWidth: true
            Layout.preferredHeight: _root.height / 8
        }
    }
}

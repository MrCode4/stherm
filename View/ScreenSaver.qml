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
    //! UiSession
    property UiSession uiSession

    //! Reference to I_DeviceController
    property I_DeviceController     deviceController: uiSession.deviceController

    //! Reference to I_Device
    property I_Device               device: deviceController?.device ?? null

    //! Unit
    property string     unit: device?.setting?.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C" ?? "F"

    property bool isActiveSchedule: deviceController?.currentSchedule ?? null

    /* Object properties
     * ****************************************************************************************/

    // ScrerenSaver popup needs to be positioned on the topmost layer for optimal visibility.
    // Although 1 technically works, 10 is chosen for redundancy as a safety measure.
    z: 10

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
                visible: deviceController.sensorsFetched
                text: !deviceController.sensorsFetched ?
                          "NAN" : Number(Utils.convertedTemperature(
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
                Layout.alignment: Qt.AlignCenter

                spacing: 10
                Image {
                    id: swUpdateIcon

                    visible: deviceController.deviceControllerCPP.system.updateAvailable
                    fillMode: Image.PreserveAspectFit
                    source: AppSpec.swUpdateIcon
                    sourceSize.width: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt
                    sourceSize.height: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt

                    cache: true
                }

                Image {
                    id: alertIcon

                    visible: uiSession.hasUnreadAlerts
                    fillMode: Image.PreserveAspectFit
                    source: AppSpec.alertIcon
                    sourceSize.width: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt
                    sourceSize.height: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt

                    cache: true
                }

                Image {
                    id: messageIcon

                    visible: uiSession.hasUnreadMessages
                    fillMode: Image.PreserveAspectFit
                    source: AppSpec.messageIcon
                    sourceSize.width: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt
                    sourceSize.height: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt

                    cache: true
                }
            }

            Image {
                visible: device._lock.isLock
                Layout.alignment: Qt.AlignHCenter
                width: 37
                height: 49
                source: "qrc:/Stherm/Images/lock-image.svg"
                fillMode: Image.PreserveAspectFit
                horizontalAlignment: Image.AlignHCenter
                verticalAlignment: Image.AlignVCenter
            }
        }

        //! Spacer
        Item {
            height: 120
            Layout.fillWidth: true
        }
    }

    //! Organization icon
    OrganizationIcon {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 3

        appModel: _root.device
        width: parent.width * 0.5
        height: parent.height * 0.25
    }
}

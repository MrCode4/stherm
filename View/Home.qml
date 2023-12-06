import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Home page of the application
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to UiSession
    property UiSession              uiSession

    //! UiPreferences
    property UiPreferences          uiPreferences: uiSession?.uiPreferences ?? null

    //! Reference to I_Device
    readonly property   I_Device    device: uiSession?.appModel ?? null

    //! Whether DesiredTempratureItem is being dragged
    readonly property   bool        isDragging: state === "dragging"

    //! Reference to main StackView
    required property   StackView   mainStackView

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size
    padding: 0

    /* Children
     * ****************************************************************************************/
    //! Desired temprature slider and value
    DesiredTempratureItem {
        id: _desiredTempItem
        //! This is to fix a bug with slider working when there is a popup
        enabled: !uiSession?.popupLayout?.isTherePopup
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height / 2.
        width: height * 2
        labelVisible: device?.systemSetup.systemMode !== AppSpecCPP.Off
        uiSession: _root.uiSession
    }

    //! This holds other items which gets hidden when DesiredTempratureItem is being dragged
    Item {
        id: _itemsToHide
        anchors.fill: parent
        anchors.margins: 2
        visible: opacity > 0

        //! Current temprature item
        CurrentTempratureLabel {
            id: _currentTempLbl
            anchors {
                left: parent.left
                top: parent.top
            }
            z: 1
            device: _root.uiSession.appModel
        }

        //! Wifi status
        WifiButton {
            id: _wifiBtn

            anchors {
                right: parent.right
                top: parent.top
            }
            z: 1

            onClicked: {
                //! Open WifiPage
                if (mainStackView) {
                    mainStackView.push("qrc:/Stherm/View/WifiPage.qml", {
                                           "uiSession": uiSession
                                       });
                }
            }
        }

        //! System mode button
        SystemModeButton {
            id: _systemModeBtn
            anchors {
                horizontalCenter: parent.horizontalCenter
                horizontalCenterOffset: -_desiredTempItem.labelWidth - 12
            }
            y: (_desiredTempItem.height - height) / 2 - 4
            deviceController: uiSession?.deviceController ?? null
        }

        Item {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: _dateTimeHolder.bottom
                horizontalCenterOffset: 8
            }
            width: _root.width - 140 * scaleFactor
            height: _airCondItem.implicitHeight + _dateTimeHolder.height + 40 * scaleFactor

            //! Humidity item
            CurrentHumidityButton {
                id: _currHumidLbl
                anchors {
                    left: parent.left
                    top: parent.top
                    topMargin: -8
                }
                device: _root.uiSession.appModel

                onClicked: {
                    if (mainStackView) {
                        mainStackView.push("qrc:/Stherm/View/HumidityPage.qml", {
                                                      "uiSession": Qt.binding(() => uiSession)
                                                  })
                    }
                }
            }

            //! Air condition item
            AirConditionItem {
                id: _airCondItem
                anchors {
                    right: parent.right
                    top: parent.top
                }
                condition: device.co2 < 2.9 ? 0 : device.co2 > 4 ? 2 : 1
            }

            //! Fan
            FanButton {
                id: _fanButton
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                }
            }

            //! Hold button
            HoldButton {
                id: _holdBtn
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                }
                uiSession: _root.uiSession
            }
        }

        Item {
            id: _dateTimeHolder
            y: _desiredTempItem.height + 4 * scaleFactor
            anchors.horizontalCenter: parent.horizontalCenter
            height: _dateTimeLbl.implicitHeight
            width: _dateTimeLbl.maximumWidth

            //! Date and Timer
            DateTimeLabel {
                id: _dateTimeLbl
                anchors.centerIn: parent
                is12Hour: device?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
            }
        }

        //! NEXGEN icon
        OrganizationIcon {
            id: _logo
            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
                bottomMargin: _menuButton.implicitHeight
            }
            height: _menuButton.height * 1.2
            opacity: (uiSession?.simulating ?? true) ? 0 : 1
        }

        //! Device Toggle Button
        DeviceToggleButton {
            anchors {
                horizontalCenter: _logo.horizontalCenter
                top: _logo.top
            }
            width: parent.width * 0.75

            uiSession : _root.uiSession
        }

        //! Menu button
        MenuButton {
            id: _menuButton
            anchors {
                left: parent.left
                bottom: parent.bottom
            }

            onClicked: {
                //! Push ApplicationMenu to StackView
                if (mainStackView) {
                    mainStackView.push("qrc:/Stherm/View/ApplicationMenu.qml", {
                                           "uiSession": Qt.binding(() => uiSession)
                                       });
                }
            }
        }

        //! Schedule button
        ScheduleButton {
            anchors {
                right: parent.right
                bottom: parent.bottom
            }

            onClicked: {
                //! Push ScheduleView to StackView
                if (mainStackView) {
                    mainStackView.push("qrc:/Stherm/View/ScheduleView.qml", {
                                           "uiSession": Qt.binding(() => uiSession)
                                       });
                }
            }
        }
    }


    /* States and Transitions
     * ****************************************************************************************/
    state: "idle"
    states: [
        State {
            name: "idle"
            when: !_desiredTempItem.dragging

            PropertyChanges {
                target: _desiredTempItem
                font.pointSize: Qt.application.font.pointSize * 3
                labelVerticalOffset: -8
            }

            PropertyChanges {
                target: _itemsToHide
                opacity: 1.
            }
        },

        State {
            name: "dragging"
            when: _desiredTempItem.dragging

            PropertyChanges {
                target: _desiredTempItem
                font.pointSize: Qt.application.font.pointSize * 4.8
                labelVerticalOffset: AppStyle.size / 15
            }

            PropertyChanges {
                target: _itemsToHide
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"

            //! First change
            NumberAnimation {
                targets: [_desiredTempItem, _itemsToHide]
                properties: "labelVerticalOffset,font.pointSize,opacity"
                duration: 250
            }
        }
    ]
}

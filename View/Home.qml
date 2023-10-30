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

    //! Reference to main StackView
    required property   StackView   mainStackView

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size

    /* Children
     * ****************************************************************************************/
    //! Desired temprature slider and value
    DesiredTempratureItem {
        id: _desiredTempItem
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height / 2
        width: parent.availableWidth
        labelVisible: device?.systemMode !== I_Device.SystemMode.Off
        uiSession: _root.uiSession
    }

    //! This holds other items which gets hidden when DesiredTempratureItem is being dragged
    Item {
        id: _itemsToHide
        anchors.fill: parent
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
            uiPreference: uiPreferences
        }

        //! Wifi status
        WifiButton {
            id: _wifiBtn
            anchors {
                right: parent.right
                top: parent.top
            }
            z: 1

            wifi: uiSession?.appModel?.wifi ?? null
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
                horizontalCenterOffset: -width
            }
            y: (_desiredTempItem.height - height) / 2 - 4
            deviceController: uiSession?.deviceController ?? null
        }

        //! Other items
        GridLayout {
            id: _otherItemsLay
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenterOffset: AppStyle.size / 24
            }
            y: _desiredTempItem.height - AppStyle.size / 10
            columns: 3
            rowSpacing: AppStyle.size / 12

            //! Humidity item
            CurrentHumidityLabel {
                Layout.alignment: Qt.AlignCenter
                device: _root.uiSession.appModel
            }

            //! Date and Timer
            DateTimeLabel {
                Layout.rowSpan: 2
                Layout.alignment: Qt.AlignCenter
                Layout.leftMargin: AppStyle.size / 30
                Layout.rightMargin: AppStyle.size / 30
                is12Hour: uiPreferences?.timeFormat === UiPreferences.TimeFormat.Hour12
            }

            //! Air condition item
            AirConditionItem {
                Layout.alignment: Qt.AlignCenter
            }

            //! Fan
            FanButton {
                Layout.alignment: Qt.AlignCenter
            }

            //! Hold button
            HoldButton {
                Layout.alignment: Qt.AlignCenter
            }
        }

        //! NEXGEN icon
        NexgenIcon {
            id: _logo
            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
                bottomMargin: _menuButton.implicitHeight * 1.8
            }
            width: parent.width * 0.5
            height: sourceSize.height * width / sourceSize.width
        }

        //! Device Toggle Button
        DeviceToggleButton {
            anchors {
                horizontalCenter: _logo.horizontalCenter
                top: _logo.bottom
                topMargin: AppStyle.size / 60
            }

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

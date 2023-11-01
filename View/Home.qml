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
        height: parent.height / 2.
        width: parent.availableWidth
        labelVisible: device?.systemMode !== AppSpec.SystemMode.Off
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

        //! Other items
        Item {
            id: _otherItemsLay
            anchors {
                verticalCenter: parent.verticalCenter
                horizontalCenter: parent.horizontalCenter
            }
            width: _fanButton.implicitWidth + _dateTimeHolder.width + _airCondHoldBtnLay.implicitWidth + 4
            height: Math.max(_currHumFanBtnLay.implicitHeight,
                             _dateTimeHolder.height,
                             _airCondHoldBtnLay.implicitHeight)

            ColumnLayout {
                id: _currHumFanBtnLay
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                spacing: 48 * scaleFactor

                //! Humidity item
                CurrentHumidityLabel {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    device: _root.uiSession.appModel
                }


                //! Fan
                FanButton {
                    id: _fanButton
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                }
            }

            Item {
                id: _dateTimeHolder
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                height: _dateTimeLbl.implicitHeight
                width: _dateTimeLbl.maximumWidth

                //! Date and Timer
                DateTimeLabel {
                    id: _dateTimeLbl
                    anchors.centerIn: parent
                    is12Hour: device?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
                }
            }

            ColumnLayout {
                id: _airCondHoldBtnLay
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    rightMargin: -16
                }
                spacing: 48 * scaleFactor

                //! Air condition item
                AirConditionItem {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                }

                //! Hold button
                HoldButton {
                    id: _holdBtn
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                }
            }
        }

        //! NEXGEN icon
        NexgenIcon {
            id: _logo
            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
                bottomMargin: _menuButton.implicitHeight * 1.5
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

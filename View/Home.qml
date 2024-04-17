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

    property I_DeviceController     deviceController: uiSession?.deviceController ?? null

    //! Whether DesiredTempratureItem is being dragged
    readonly property   bool        isDragging: state === "dragging"

    //! Reference to main StackView
    required property   StackView   mainStackView

    //! System Accessories use in humidity control.
    property SystemAccessories systemAccessories: device.systemSetup.systemAccessories


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
        height: parent.height / 2. + 8
        width: height * 2
        labelVisible: device?.systemSetup.systemMode !== AppSpecCPP.Off
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

            TapHandler {
                onTapped: {
                    _root.StackView.view.push("qrc:/Stherm/View/SensorsPage.qml", {
                                                                      "uiSession": Qt.binding(() => uiSession)
                                                                  })
                }
            }
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
                bottom: centerItems.top
                bottomMargin: -16
            }
            enabled: !deviceController.currentSchedule
            hoverEnabled: enabled
            deviceController: uiSession?.deviceController ?? null

            onClicked: {
                if (!deviceController.currentSchedule) {
                    _root.StackView.view.push("qrc:/Stherm/View/SystemModePage.qml", {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
                }
            }
        }

        Item {
            id: centerItems
            y: _desiredTempItem.height - _airCondItem.height
            anchors {
                horizontalCenter: parent.horizontalCenter
                horizontalCenterOffset: 8
            }
            width: _root.width - 120
            height: _airCondItem.implicitHeight + _dateTimeHolder.height + 16 * scaleFactor

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
                    if (mainStackView && (systemAccessories.accessoriesWireType !== AppSpecCPP.None)) {
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
                // using iaq
                condition: device._co2_id
            }

            //! Fan
            FanButton {
                id: _fanButton

                deviceController: uiSession.deviceController
                appModel: uiSession.appModel

                anchors {
                    left: parent.left
                    bottom: parent.bottom
                }

                onClicked: {
                    _root.StackView.view.push("qrc:/Stherm/View/FanPage.qml",
                                              {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
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
            y: _desiredTempItem.height + 12 * scaleFactor
            anchors.horizontalCenter: parent.horizontalCenter
            height: _dateTimeLbl.implicitHeight
            width: _dateTimeLbl.maximumWidth

            //! Date and Timer
            DateTimeLabel {
                id: _dateTimeLbl
                anchors.centerIn: parent
                is12Hour: device?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
            }

            TapHandler {
                onTapped: {
                    uiSession.popupLayout.displayPopUp(timeFormatPop, true);
                }
            }
        }

        OnScheduleLabel {
            anchors {
                bottom: _dateTimeHolder.top
                horizontalCenter: _dateTimeHolder.horizontalCenter
                bottomMargin: 16
            }
            visible: uiSession.deviceController.currentSchedule
            font {
                pointSize: _root.font.pointSize * 0.8
            }

            TapHandler {
                onTapped: {
                    if (mainStackView) {
                        mainStackView.push("qrc:/Stherm/View/ScheduleView.qml", {
                                               "uiSession": Qt.binding(() => uiSession)
                                           });
                    }
                }
            }
        }

        //! NEXGEN icon
        OrganizationIcon {
            id: _logo

            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 5

            appModel: _root.device
            width: parent.width * 0.5
            height: parent.height * 0.25

            TapHandler {
                onTapped: {
                    mainStackView.push("qrc:/Stherm/View/ContactContractorPage.qml", {
                                           "uiSession": _root.uiSession
                                       });
                }
            }
        }

        //! Menu button
        MenuButton {
            id: _menuButton
            anchors {
                left: parent.left
                bottom: parent.bottom
            }

            hasNotification: uiSession.hasUpdateNotification
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

    TimeFormatPopup {
        id: timeFormatPop
        uiSession: _root.uiSession
    }


    //! Open a page from home
    Connections {
        target: uiSession.popUps

        function onOpenPageFromHome(item: string) {
            if (mainStackView) 
                mainStackView.push(item, {
                                          "uiSession": Qt.binding(() => uiSession)
                                      });
        }
    }

    Connections {
        target: deviceController.deviceControllerCPP

        enabled: !uiSession.debug

        function onStartModeChanged(startMode: int) {
            // Temp
            deviceController.startMode = startMode;

            //! Open a test mode page from home when app start with test mode.
            if (startMode === 0) {
                uiSession.uiTetsMode = true;
                if (mainStackView)
                    mainStackView.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                           "uiSession": Qt.binding(() => uiSession)
                                       });

            } else {
                deviceController.setInitialSetup(true);
                //! Open WifiPage
                if (mainStackView) {
                    mainStackView.push("qrc:/Stherm/View/WifiPage.qml", {
                                           "uiSession": uiSession,
                                           "backButtonVisible": false,
                                           "initialSetup": true
                                       });
                }
            }
       }
    }

    Timer {
        id: startupTimer
        repeat: false
        running: false
        interval: 100

        onTriggered: {
            // Active the screen saver
            ScreenSaverManager.setActive();

            uiSession.showHome();

            // Send  check contractor info
            deviceController.deviceControllerCPP.checkContractorInfo();

            deviceController.setInitialSetup(false);
        }
    }

    //! Check SN mode
    Connections {
        id: startupSN
        target: deviceController.deviceControllerCPP

        function onSnModeChanged(snMode: bool) {
            // snMode != 2
            if (snMode) {
                // should be done by timer as can cause crash
                startupTimer.start()
                // disable fetching sn again
                startupSN.enabled = false;
                snChecker.enabled = false;
            }
        }
    }

    //! checkSN when the internet is connected.
    Connections {
        id: snChecker
        target: NetworkInterface

        function onHasInternetChanged() {
            if (NetworkInterface.hasInternet) {
                if (deviceController.startMode !== 0 && deviceController.startMode !== -1){
                    deviceController.deviceControllerCPP.checkSN();
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
                labelVerticalOffset: device?.systemSetup?.systemMode === AppSpec.Auto ? -8 : -32
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

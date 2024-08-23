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

    property System system: deviceController.deviceControllerCPP.system

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
        TempratureLabel {
            id: _currentTempLbl
            anchors {
                left: parent.left
                top: parent.top
            }
            z: 1
            device: _root.uiSession.appModel
            visible: deviceController.sensorsFetched

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
            y: 116
            anchors {
                horizontalCenter: parent.horizontalCenter
                // To align with schedule ON button
                horizontalCenterOffset: isSchedule ? -15 : -6
            }

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
            y: _desiredTempItem.height - _airCondItem.height + 8
            anchors {
                horizontalCenter: parent.horizontalCenter
                horizontalCenterOffset: 12
            }
            width: _root.width - 120
            height: _airCondItem.implicitHeight + _dateTimeHolder.height + 34 * scaleFactor

            RowLayout {
                x: -4
                width: parent.width - 4
                spacing: 10

                //! Humidity item
                CurrentHumidityButton {
                    id: _currHumidLbl
                    Layout.alignment: Qt.AlignLeading
                    Layout.leftMargin: -8
                    device: _root.uiSession.appModel

                    visible: deviceController.sensorsFetched

                    onClicked: {
                        if (mainStackView && (systemAccessories.accessoriesWireType !== AppSpecCPP.None)) {
                            mainStackView.push("qrc:/Stherm/View/HumidityPage.qml", {
                                                   "uiSession": Qt.binding(() => uiSession)
                                               })
                        }
                    }
                }

                Item {
                    Layout.alignment: Qt.AlignHCenter
                    implicitWidth: scheduleOnLbl.implicitWidth
                    implicitHeight: scheduleOnLbl.implicitHeight

                    OnScheduleLabel {
                        id: scheduleOnLbl
                        anchors.fill: parent
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
                }

                //! Air condition item
                AirConditionItem {
                    id: _airCondItem
                    Layout.alignment: Qt.AlignTrailing
                    // using iaq
                    condition: device._co2_id
                    visible: deviceController.sensorsFetched && deviceController.airConditionSensorHealth
                }
            }

            //! Fan
            FanButton {
                id: _fanButton

                deviceController: uiSession.deviceController
                appModel: uiSession.appModel

                anchors {
                    left: parent.left
                    bottom: parent.bottom
                    leftMargin: -8
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
                    rightMargin: 8
                }
                uiSession: _root.uiSession
            }
        }

        Item {
            id: _dateTimeHolder
            anchors {
                bottom: centerItems.bottom
                horizontalCenter: parent.horizontalCenter
                bottomMargin: 8
                horizontalCenterOffset: -8
            }
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

        //! NEXGEN icon
        OrganizationIcon {
            id: _logo

            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 5

            appModel: _root.device
            width: parent.width * 0.5
            height: parent.height * 0.2

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
                                       }, StackView.Immediate);

                }
            }
        }

        //! Schedule button
        ScheduleButton {
            anchors {
                right: parent.right
                bottom: parent.bottom
            }

            uiSession: _root.uiSession
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
            if (uiSession.uiTestMode)
                return;

            // Temp
            deviceController.startMode = startMode;

            //! Open a test mode page from home when app start with test mode.
            if (startMode === 0) {
                uiSession.uiTestMode = true;
                if (mainStackView) {

                    // If a specific version has been accepted only once in test mode,
                    // display the VersionInformationPage.
                    // Otherwise, display the PrivacyPolicyPage for acceptance.
                    if (device.userPolicyTerms.acceptedVersionOnTestMode === device.userPolicyTerms.currentVersion) {
                        mainStackView.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession),
                                                     "backButtonVisible" : false
                                                 });
                    } else {
                        mainStackView.push("qrc:/Stherm/View/PrivacyPolicyPage.qml", {
                                               "uiSession": Qt.binding(() => uiSession),
                                               "testMode" : true
                                           });
                    }
                }

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

            // Settings fetch from server at least once before show home
            // Wait for WIFI page to fetch settings and show home here.
            if(!uiSession.settingsReady) {
                interval = 4000;
                restart();

                return;
            }

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

        function onSnModeChanged(snMode: int) {
            // snMode === 1 or 0
            var snTestMode = deviceController.deviceControllerCPP.getSNTestMode();
            if (snMode !== 2 || snTestMode) {

                //! Setting is ready in device or not
                if (!uiSession.settingsReady)
                    uiSession.settingsReady = (snMode === 0);

                // should be done by timer as can cause crash
                startupTimer.start()
                // disable fetching sn again
                startupSN.enabled = false;
                snChecker.enabled = false;

            }
        }
    }

    //! Force the app to fetch again with new serial number
    Connections {
        target: system

        function onSerialNumberChanged() {
            console.log("initialSetup (in onSerialNumberChanged slot): ", deviceController.initialSetup)
            uiSession.settingsReady = false;
        }

        function onTestModeStarted() {
            if (uiSession.uiTestMode)
                return;
            console.log("Test mode started due to serial number issues.")
            uiSession.uiTestMode = true;
            deviceController.startMode = 0;
            deviceController.testModeType = AppSpec.TestModeType.SerialNumber;
            system.ignoreManualUpdateMode();

            if (mainStackView)
                mainStackView.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                       "uiSession": Qt.binding(() => uiSession),
                                       "backButtonVisible" : false
                                   });
        }
    }

    //! checkSN when the internet is connected.
    Connections {
        id: snChecker
        target: NetworkInterface

        function onHasInternetChanged() {
            if (NetworkInterface.hasInternet) {
                if (deviceController.startMode !== 0 && deviceController.startMode !== -1) {
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
                font.pointSize: Qt.application.font.pointSize * 2.8
                labelVerticalOffset: device?.systemSetup?.systemMode === AppSpec.Auto
                                     && !_desiredTempItem.currentSchedule ? -12 : -28
                enableAnimations: true
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
                labelVerticalOffset: _desiredTempItem.height / 2
                enableAnimations: false
            }

            PropertyChanges {
                target: _itemsToHide
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            reversible: true
            from: "idle"
            to: "dragging"

            SequentialAnimation {
                PropertyAnimation {
                    target: _desiredTempItem
                    property: "enableAnimations"
                    duration: 0
                }

                NumberAnimation {
                    targets: [_desiredTempItem, _itemsToHide]
                    properties: "labelVerticalOffset,font.pointSize,opacity"
                    duration: 250
                }
            }
        }
    ]
}

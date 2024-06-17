import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ApplicationMenu provides ui to access different section of application
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    backButtonVisible: false
    header: null
    visible: false

    onVisibleChanged: {
        if (visible) {
            innerPage.y = 0
            innerPage.opacity = 1.0
        }
    }

    /* Children
     * ****************************************************************************************/

    BasePageView {
        id: innerPage

        /* Object properties
         * ****************************************************************************************/

        backButtonCallback: function() {
            if (root.StackView.view) {
                //! Then Page is inside an StackView
                if (root.StackView.view.currentItem === root) {
                    root.StackView.view.pop();
                }
            }
        }

        title: "Menu"
        y: height
        opacity: 0

        //! Attach the animations
        //! Behaviour on y
        Behavior on y {
            NumberAnimation {
                from: height
                to: 0
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        //! Behaviour on opacity
        Behavior on opacity  {
            NumberAnimation {
                from: 0.0
                to: 1.0
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        contentItem: ApplicationMenuList {
            uiSession: root.uiSession

            onMenuActivated: function(menuTitle) {
                //! Push related menu to stack
                if (root.StackView.view) {
                    switch(menuTitle) {
                    case "System Mode":
                        root.StackView.view.push("qrc:/Stherm/View/SystemModePage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Sensors":
                        root.StackView.view.push("qrc:/Stherm/View/SensorsPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 })
                        break
                    case "Alerts/Messages":
                        root.StackView.view.push("qrc:/Stherm/View/AlertsNotificationsPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Backlight":
                        root.StackView.view.push("qrc:/Stherm/View/BacklightPage.qml",
                                                 {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Fan Control":
                        root.StackView.view.push("qrc:/Stherm/View/FanPage.qml",
                                                 {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Schedule":
                        root.StackView.view.push("qrc:/Stherm/View/ScheduleView.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Humidity Control":
                        root.StackView.view.push("qrc:/Stherm/View/HumidityPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Settings":
                        root.StackView.view.push("qrc:/Stherm/View/SettingsPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Date & Time":
                        root.StackView.view.push("qrc:/Stherm/View/DateTime/DateTimePage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "System Setup":
                        root.StackView.view.push("qrc:/Stherm/View/SystemSetupPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Wi-Fi Settings":
                        root.StackView.view.push("qrc:/Stherm/View/WifiPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;
                    case "Device Information":
                        root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 })
                        break;
                    case "Technician Access":
                        root.StackView.view.push("qrc:/Stherm/View/UserGuidePage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 })
                        break;
                    case "System Info":
                        root.StackView.view.push("qrc:/Stherm/View/SystemInfoPage.qml")
                        break;
                    case "Contact Contractor":
                        root.StackView.view.push("qrc:/Stherm/View/ContactContractorPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;

                    case "System Update":
                        root.StackView.view.push("qrc:/Stherm/View/SystemUpdatePage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                        break;

                    case "System Update Stage":{
                        uiSession.uiTestMode = true;
                        deviceController.deviceControllerCPP.system.testMode = true;
                        root.StackView.view.push("qrc:/Stherm/View/SystemUpdatePage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                    } break;

                    case "Test Mode": {
                        uiSession.uiTestMode = true;
                        root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                    } break;

                    case "Night Mode": {
                        root.StackView.view.push("qrc:/Stherm/View/NightModePage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                    } break;
                    }
                }
            }
        }
    }
}

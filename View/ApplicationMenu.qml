import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ApplicationMenu provides ui to access different section of application
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! System
    property System system: deviceController.deviceControllerCPP.system

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

        property var menuModel: [
            {
                icon: FAIcons.sunDust,
                text: "System Mode",
                view: "qrc:/Stherm/View/SystemModePage.qml"
            },
            {
                icon: FAIcons.bolt,
                text: "Backlight",
                view: "qrc:/Stherm/View/BacklightPage.qml"
            },
            {
                icon: FAIcons.bellExclamation,
                text: "Alerts/Messages",
                view: "qrc:/Stherm/View/AlertsNotificationsPage.qml"
            },
            {
                icon: FAIcons.calendarDays,
                text: "Schedule",
                view: "qrc:/Stherm/View/ScheduleView.qml"
            },
            {
                icon: FAIcons.gear,
                text: "Settings",
                view: "qrc:/Stherm/View/SettingsPage.qml"
            },
            {
                icon: FAIcons.calendarClock,
                text: "Date & Time",
                view: "qrc:/Stherm/View/DateTime/DateTimePage.qml"
            },
            {
                icon: FAIcons.signalStream,
                text: "Sensors",
                view: "qrc:/Stherm/View/SensorsPage.qml"
            },
            {
                icon: FAIcons.droplet,
                text: "Humidity Control",
                visible: ((systemAccessories?.accessoriesWireType ?? AppSpecCPP.None) !== AppSpecCPP.None),
                view: "qrc:/Stherm/View/HumidityPage.qml"
            },
            {
                icon: FAIcons.fan,
                text: "Fan Control",
                view: "qrc:/Stherm/View/FanPage.qml"
            },
            {
                icon: FAIcons.wrench,
                text: "System Setup",
                view: "qrc:/Stherm/View/SystemSetupPage.qml"
            },
            {
                image: "qrc:/Stherm/Images/smart-phone.png",
                text: "Mobile App",
                view: "qrc:/Stherm/View/MobileAppPage.qml"
            },
            {
                icon: FAIcons.wifi,
                text: "Wi-Fi Settings",
                view: "qrc:/Stherm/View/WifiPage.qml"
            },
            {
                icon: FAIcons.lock,
                text: "Lock",
                view: "qrc:/Stherm/View/LockPage.qml"
            },
            {
                icon: FAIcons.night,
                text: "Night Mode",
                visible: false,
                view: "qrc:/Stherm/View/NightModePage.qml"
            },
            {
                icon: FAIcons.fileExclamation,
                text: "Technician Access",
                view: "qrc:/Stherm/View/UserGuidePage.qml"
            },
            {
                icon: FAIcons.arrowsRotate,
                text: "System Update",
                view: "qrc:/Stherm/View/SystemUpdatePage.qml",
                hasNotification: uiSession.hasUpdateNotification,
                shouldGo: function () {
                    if (system.isFWServerUpdate()) {
                        uiSession.toastManager.showToast("System update are currently unavailable", "due to firmware server update.");
                        return false;
                    }
                    return true;
                },
                longPressAction: function() {
                    uiSession.uiTestMode = true;
                    system.testMode = true;
                    root.StackView.view.push("qrc:/Stherm/View/SystemUpdatePage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession)
                                             });
                }
            },
            {
                icon: FAIcons.memoCircleInfo,
                text: "Device Information",
                view: "qrc:/Stherm/View/AboutDevicePage.qml",
                longPressAction: function() {
                    uiSession.uiTestMode = true;
                    root.StackView.view.push("qrc:/Stherm/View/SensorsPage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession)
                                             });
                }
            },
            {
                icon: FAIcons.headSet,
                text: "Contact Contractor",
                view: "qrc:/Stherm/View/ContactContractorPage.qml"
            },
            // {
            //     icon: FAIcons.memoCircleInfo,
            //     text: "System Info"
            // }
        ]

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
                        if (system.isFWServerUpdate()) {
                            uiSession.toastManager.showToast("System update are currently unavailable", "due to firmware server update.");

                        } else {
                            root.StackView.view.push("qrc:/Stherm/View/SystemUpdatePage.qml", {
                                                         "uiSession": Qt.binding(() => uiSession)
                                                     });
                        }


                        break;

                    case "System Update Stage":{
                        uiSession.uiTestMode = true;
                        system.testMode = true;
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
                    case "Mobile App": {
                        root.StackView.view.push("qrc:/Stherm/View/MobileAppPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                    } break;
                    case "Lock": {
                        root.StackView.view.push("qrc:/Stherm/View/LockPage.qml", {
                                                     "uiSession": Qt.binding(() => uiSession)
                                                 });
                    } break;
                    }
                }
            }
        }
    }
}

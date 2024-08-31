import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

BasePageView {
    id: root    
    title: "Settings"

    property System system: deviceController.deviceControllerCPP.system
    property bool showHiddenItems: false
    onTitleLongTapped: root.showHiddenItems = true
    backButtonCallback: {
        if (root.showHiddenItems) {
            root.showHiddenItems = false;
        }
        else {
            tryGoBack();
        }
    }

    contentItem: MenuListView {
        onMenuActivated: function(item) {
            let newProps = {};
            Object.assign(newProps, item.props);
            Object.assign(newProps, {"uiSession": Qt.binding(() => uiSession)});
            root.StackView.view.push(item.view, newProps);
        }

        property var commonItems: [
            {
                icon: FAIcons.gear,
                text: "General",
                view: "qrc:/Stherm/View/SettingsPage.qml"
            },
            {
                icon: FAIcons.calendarClock,
                text: "Date & Time",
                view: "qrc:/Stherm/View/DateTime/DateTimePage.qml"
            },
            {
                icon: FAIcons.bellExclamation,
                text: "Alerts",
                view: "qrc:/Stherm/View/Menu/AlertsPage.qml"
            },
            {
                icon: FAIcons.signalStream,
                text: "Sensors",
                view: "qrc:/Stherm/View/SensorsPage.qml"
            },
            {
                icon: FAIcons.droplet,
                text: "Humidity Control",
                visible: ((root.appModel?.systemSetup?.systemAccessories?.accessoriesWireType ?? AppSpecCPP.None) !== AppSpecCPP.None),
                view: "qrc:/Stherm/View/HumidityPage.qml"
            },
            {
                icon: FAIcons.fan,
                text: "Fan Control",
                view: "qrc:/Stherm/View/FanPage.qml"
            },
            {
                icon: FAIcons.arrowsRotate,
                text: "System Update",
                view: "qrc:/Stherm/View/SystemUpdatePage.qml",
                hasNotification: uiSession.hasUpdateNotification,
                prepareAndCheck: function () {
                    if (system.isFWServerUpdate()) {
                        uiSession.toastManager.showToast("System update are currently unavailable", "due to firmware server update.");
                        return false;
                    }
                    return true;
                },
                longPressAction: function() {
                    uiSession.uiTestMode = true;
                    system.testMode = true;
                    root.StackView.view.push("qrc:/Stherm/View/SystemUpdatePage.qml", {"uiSession": Qt.binding(() => uiSession)});
                }
            },
            {
                icon: FAIcons.memoCircleInfo,
                text: "Device Information",
                view: "qrc:/Stherm/View/AboutDevicePage.qml",
                props: {showTestMode: true}
            },
            {
                image: "qrc:/Stherm/Images/smart-phone.png",
                text: "Mobile App",
                view: "qrc:/Stherm/View/MobileAppPage.qml"
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
            }
        ]

        property var hiddenItems: [
            {isSeparator: true},
            {
                icon: FAIcons.fileExclamation,
                text: "Technician Access",
                color: Style.hiddenMenuColor,
                view: "qrc:/Stherm/View/UserGuidePage.qml"
            },
            {
                icon: FAIcons.wrench,
                text: "System Setup",
                color: Style.hiddenMenuColor,
                view: "qrc:/Stherm/View/SystemSetupPage.qml"
            },
            {
                icon: FAIcons.bellExclamation,
                text: "System Alerts",
                color: Style.hiddenMenuColor,
                view: "qrc:/Stherm/View/Menu/SystemAlertsPage.qml"
            }
        ]

        model: root.showHiddenItems ? commonItems.concat(hiddenItems) : commonItems
    }
}

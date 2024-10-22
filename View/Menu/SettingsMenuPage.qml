import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import Ronia
import Stherm

BasePageView {
    id: root    
    title: "Settings"

    property System system: deviceController.deviceControllerCPP.system
    property bool showHiddenItems: false

    enableTitleTap: true
    onTitleLongTapped: root.showHiddenItems = true

    //! Last item to be easily touchable
    bottomPadding: 20

    backButtonCallback: () => {
        if (root.showHiddenItems) {
            root.showHiddenItems = false;
        }
        else {
            tryGoBack();
        }
    }

    contentItem: MenuListView {
        onCountChanged: {
            if (root.showHiddenItems && count == model.length) {
                positionViewAtEnd();
            }
        }

        onMenuActivated: function(item) {
            if (item.view) {
                let newProps = {};
                Object.assign(newProps, item.props);
                Object.assign(newProps, {"uiSession": Qt.binding(() => uiSession)});
                root.StackView.view.push(item.view, newProps);
            }
            else if(item.action instanceof Function) {
                item.action();
            }
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
                icon: FAIcons.cloud_arrow_down,
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
                icon: FAIcons.circleInfo,
                text: "Device Information",
                view: "qrc:/Stherm/View/AboutDevicePage.qml",
                props: {showTestMode: root.showHiddenItems}
            },
            {
                icon: FAIcons.night,
                text: "Night Mode",
                visible: false,
                view: "qrc:/Stherm/View/NightModePage.qml"
            },
            {
                icon: FAIcons.screwdriver_wrench,
                text: "Technician Access",
                view: "qrc:/Stherm/View/UserGuidePage.qml",
                visible: false
            },
            {
                icon: FAIcons.sliders,
                text: "System Setup",
                view: "qrc:/Stherm/View/SystemSetupPage.qml"
            }
        ]

        property var hiddenItems: [
            {isSeparator: true},            
            {
                icon: FAIcons.triangleExclamation,
                text: "System Alerts",
                color: Style.hiddenMenuColor,
                view: "qrc:/Stherm/View/Menu/SystemAlertsPage.qml"
            },
            {
                icon: FAIcons.circleCheck,
                text: "Perf Test",
                color: Style.hiddenMenuColor,
                action: () => {
                    popupPerfTest.message = "Checking Eligibility";
                    popupPerfTest.isBusyMode = true;
                    popupPerfTest.open();
                    PerfTestService.eligibilityChecked.connect(root.onPerfTestEligibilityChecked);
                    PerfTestService.checkTestEligibility();
                }
            }
        ]

        model: root.showHiddenItems ? commonItems.concat(hiddenItems) : commonItems
    }

    function onPerfTestEligibilityChecked(errMsg) {
        PerfTestService.eligibilityChecked.disconnect(root.onPerfTestEligibilityChecked);
        if (!errMsg) {
            popupPerfTest.close();
            uiSession.showHome();
        }
        else {
            popupPerfTest.message = errMsg;
            popupPerfTest.isBusyMode = false;
        }
    }

    Popup {
        id: popupPerfTest
        parent: T.Overlay.overlay
        width: parent.width * 0.6
        anchors.centerIn: parent
        modal: true

        property bool isBusyMode: true
        property string message: "Checking Eligibility"

        contentItem: ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 10
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                font.pointSize: root.font.pointSize * 0.7
                horizontalAlignment: Qt.AlignHCenter
                wrapMode: Text.WordWrap
                text: popupPerfTest.message
            }

            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                height: 45
                width: 45
                running: visible
                visible: popupPerfTest.isBusyMode
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                visible: !popupPerfTest.isBusyMode
                onClicked: popupPerfTest.close()
                text: "OK"
            }
        }
    }
}

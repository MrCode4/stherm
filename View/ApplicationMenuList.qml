import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ApplicationMenuList is a ListView to show application settings
 * ***********************************************************************************************/
ListView {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal menuActivated(string menu)

    /* Property declaration
     * ****************************************************************************************/

    property I_Device appModel

    property I_DeviceController deviceController

    //! System, use in update notification
    property System             system:            deviceController.deviceControllerCPP.system

    property bool               hasUpdateNotification: system.updateAvailable

    //! SystemAccessories
    property SystemAccessories  systemAccessories: appModel?.systemSetup?.systemAccessories ?? null

    /* Object properties
     * ****************************************************************************************/
    ScrollIndicator.vertical: ScrollIndicator {}

    implicitWidth: 480
    implicitHeight: contentHeight
    clip: true
    model: [
        {
            "icon": FAIcons.sunDust, //! From FontAwesome
            "text": "System Mode"
        },
        {
            "icon": FAIcons.bolt,
            "text": "Backlight"
        },
        {
            "icon": FAIcons.bellExclamation,
            "text": "Alerts/Notifications"
        },
        {
            "icon": FAIcons.calendarDays,
            "text": "Schedule"
        },
        {
            "icon": FAIcons.gear,
            "text": "Settings"
        },
        {
            "icon": FAIcons.calendarClock,
            "text": "Time Settings"
        },
        {
            "icon": FAIcons.signalStream,
            "text": "Sensors"
        },
        {
            "icon": FAIcons.droplet,
            "text": "Humidity Control",
            "visible": ((systemAccessories?.accessoriesWireType ?? AppSpecCPP.None) !== AppSpecCPP.None)
        },
        {
            "icon": FAIcons.fan,
            "text": "Fan Control"
        },
        {
            "icon": FAIcons.wrench,
            "text": "System Setup"
        },
        {
            "icon": FAIcons.fileExclamation,
            "text": "Technician Access"
        },
        {
            "icon": FAIcons.arrowsRotate,
            "text": "System Update",
            "hasNotification": root.hasUpdateNotification
        },
        {
            "icon": FAIcons.memoCircleInfo,
            "text": "Device Information"
        },
        {
            "icon": FAIcons.headSet,
            "text": "Contact Contractor"
        },
        // {
        //     "icon": FAIcons.memoCircleInfo,
        //     "text": "System Info"
        // }
    ]
    delegate: ApplicationMenuDelegate {
        width: ListView.view.width
        height: (modelData?.visible ?? true) ? implicitHeight : 0
        text: delegateData?.text ?? ""
        delegateData: modelData
        delegateIndex: index

        visible: modelData?.visible ?? true

        hasNotification:  modelData?.hasNotification ?? false

        onClicked: {
            root.menuActivated(delegateData.text);
            root.hasUpdateNotification = false;
        }

        //! Show test mode on "Device Information" button
        MouseArea {
            anchors.fill: parent
            enabled: parent.text === "Device Information"
            propagateComposedEvents: true;

            pressAndHoldInterval: 10000

            onClicked: {
                root.menuActivated(parent.text);
            }

            onPressAndHold: {
                root.menuActivated("Test Mode");
            }
        }
    }

    //! Manage update notifications ()
    Connections {
        target: system

        function onUpdateAvailableChanged() {
            root.hasUpdateNotification = system.updateAvailable;
        }
    }
}

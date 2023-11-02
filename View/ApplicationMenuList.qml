import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ApplicationMenuList is a ListView to show application settings
 * ***********************************************************************************************/
ListView {
    id: _root

    /* Signals
     * ****************************************************************************************/
    signal menuActivated(string menu)

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 480
    implicitHeight: contentHeight
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}
    model: [
        {
            "icon": FAIcons.sunDust, //! From FontAwesome
            "text": "System Mode"
        },
/*        {
            "icon": FAIcons.triangleExclamation,
            "text": "Alerts"
        },*/
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
            "icon": FAIcons.signalStream,
            "text": "Sensors"
        },
        {
            "icon": FAIcons.droplet,
            "text": "Humidity Control"
        },
        {
            "icon": FAIcons.fan,
            "text": "Fan Control"
        },
        {
            "icon": FAIcons.circleCheck,
            "text": "Wiring"
        },
        {
            "icon": FAIcons.wrench,
            "text": "System Setup"
        },
        {
            "icon": FAIcons.memoCircleInfo,
            "text": "System Info"
        }
    ]
    delegate: ApplicationMenuDelegate {
        width: ListView.view.width
        text: delegateData?.text ?? ""
        delegateData: modelData
        delegateIndex: index

        onClicked: {
            menuActivated(delegateData.text);
        }
    }
}

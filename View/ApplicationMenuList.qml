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
            "icon": "", //! From FontAwesome
            "text": "System Mode"
        },
        {
            "icon": "",
            "text": "Alerts"
        },
        {
            "icon": "",
            "text": "Backlight"
        },
        {
            "icon": "",
            "text": "Alerts/Notifications"
        },
        {
            "icon": "",
            "text": "Schedule"
        },
        {
            "icon": "",
            "text": "Settings"
        },
        {
            "icon": "",
            "text": "Sensors"
        },
        {
            "icon": "",
            "text": "Humidity Control"
        },
        {
            "icon": "",
            "text": "Fan Control"
        },
        {
            "icon": "",
            "text": "Wiring"
        },
        {
            "icon": "",
            "text": "System Setup"
        },
        {
            "icon": "",
            "text": "System Info"
        }
    ]
    delegate: ApplicationMenuDelegate {
        width: ListView.view.width
        height: Material.delegateHeight
        text: delegateData?.text ?? ""
        delegateData: modelData
        delegateIndex: index

        onClicked: {
            menuActivated(delegateData.text);
        }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ForgetWifiDialog
 * ***********************************************************************************************/
I_Dialog {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Wifi name (ssid)
    property string     wifiSsid: ""

    /* Object properties
     * ****************************************************************************************/
    title: `Forget Wi-Fi Network "${wifiSsid}"?`
    description: "Your device will no longer join this Wi-Fi network automatically."
    buttons: I_Dialog.DialogButton.Cancel | I_Dialog.DialogButton.Yes

    Component.onCompleted: {
        setButtonText(I_Dialog.DialogButton.Yes, "Forget");
    }
}

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
    title: "Are you sure to reset settings?"
    description: "Your device will be reset to factory settings."
    buttons: I_Dialog.DialogButton.No | I_Dialog.DialogButton.Yes
}

import QtQuick
import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *
 * ************************************************************************************************/
QSObject {

    //! Array of wifi devices
    //! Array<WifiDevice>
    property var devices: []

    //! Holds ssid of connected device
    property string connectedSsid: NetworkInterface.connectedSsid
}

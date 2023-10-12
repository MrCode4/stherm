import QtQuick
import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *
 * ************************************************************************************************/
QSObject {

    required property WifiDevice device

    //! Connect to a device
    function connect(uuid: string): Boolean {
        return false;
    }

    //! Disconnect
    function disconnect(uuid: string): Boolean {
        return false;

    }

    //! Forget a device
    function forget(uuid: string): void {

    }

}

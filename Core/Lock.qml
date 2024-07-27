import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *  Lock: Lock model
 * ************************************************************************************************/

QSObject {
    //! PIN to lock/unlock the app
    property string pin: ""

    //! Is the app currently locked?
    property bool isLock: false
}

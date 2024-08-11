import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *  Lock: Lock model
 * ************************************************************************************************/

QSObject {
    //! PIN to lock/unlock the app
    property string pin: ""

    property string _masterPIN: ""

    //! Is the app currently locked?
    property bool isLock: false
}

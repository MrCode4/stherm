import QtQuick

import QtQuickStream

/*! ***********************************************************************************************
 * Wiring model holds info about current wiring state of the device
 * \todo: this should be fetched and set from device
 * ***********************************************************************************************/
QSObject {
    id: _root

    property bool       isR     : false

    property bool       isC     : false

    property bool       isG     : false

    property bool       isY1    : false

    property bool       isY2    : false

    property bool       isT2    : false

    property bool       isW1    : false

    property bool       isW2    : false

    property bool       isW3    : false

    property bool       isOB    : false

    property bool       isT1p   : false

    property bool       isT1n   : false
}

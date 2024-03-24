import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * NightMode keep the night mode model
 * ************************************************************************************************/

QSObject {

    property bool _running: false

    //! Night Mode
    property int mode:             AppSpec.NMOff
}

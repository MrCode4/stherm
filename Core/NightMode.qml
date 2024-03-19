import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * NightMode keep the night mode model
 * ************************************************************************************************/

QSObject {

    //! Night Mode
    property int mode:             AppSpec.NMOff

    property string startTime: "09:00 PM"

    property string endTime:    "07:00 AM"
}

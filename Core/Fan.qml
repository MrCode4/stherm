import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *
 * ************************************************************************************************/
QSObject {

    //! Fan Mode
    property int mode:             AppSpec.FMAuto

    //! Fan working period per each hour
    property int workingPerHour: 30
}

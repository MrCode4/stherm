import QtQuick
import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *
 * ************************************************************************************************/
QSObject {

    //! Fan Mode
    property int mode:             AppSpec.FanMode.FMAuto

    //! Fan working period per each hour
    property int working_per_hour: 30
}

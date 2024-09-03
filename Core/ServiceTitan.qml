import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *  ServiceTitan: service titan model
 * ************************************************************************************************/

QSObject {
    //! Was the ServiceTitan data fetch successful?
    property bool   _fetched:       false

    property bool   isSTManualMode: false

    property bool   isActive:       false

    property string email:          ""

    property string zipCode:        ""

    property string jobNumber:      ""

    property string fullName:       ""
}

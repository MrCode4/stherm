pragma Singleton

import QtQuick
import QtQuickStream

/*! ***********************************************************************************************
 * AppCore....
 * ************************************************************************************************/
QSCore {
    id: core

    /* Property Declarations
     * ****************************************************************************************/
    property I_Device model: SimDevice {
        _qsRepo: defaultRepo
    }

    property QtObject _internal: QtObject {
        readonly property var imports: [ "QtQuickStream", "Stherm"]
    }

    /* Object Properties
     * ****************************************************************************************/
    defaultRepo: createDefaultRepo(_internal.imports);


    /* Functions
     * ****************************************************************************************/
}

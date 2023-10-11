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
    property I_Model model: RootModel {
        _qsRepo: defaultRepo
    }

    property QtObject _internal: QtObject {
        readonly property var imports: [ "QtQuickStream", "QTemplateProject"]
    }

    /* Object Properties
     * ****************************************************************************************/
    defaultRepo: createDefaultRepo(_internal.imports);


    /* Functions
     * ****************************************************************************************/
}

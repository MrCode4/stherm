import QtQuick
import QtQuickStream
import QTemplateProject

/*! ***********************************************************************************************
 *
 * ************************************************************************************************/
QSObject {
    id: appModel

    /* Property Declarations
     * ****************************************************************************************/

    //! Selection Model
    property SelectionModel selectionModel: SelectionModel {}

    //! Adding a sample device
    Component.onCompleted: {
    }

    /* Functions
     * ****************************************************************************************/
}

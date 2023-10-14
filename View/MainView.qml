import QtQuick
import QtQuick.Layouts
import Stherm

/*! ***********************************************************************************************
 * The central widget of the application top level gui.
 *
 * ************************************************************************************************/
Item {
    id: mainView

    /* Property Declarations
     * ****************************************************************************************/
    property UiSession  uiSession

    /* Children
     * ****************************************************************************************/
    Home {
        anchors.fill: parent
        uiSession: mainView.uiSession
    }
}

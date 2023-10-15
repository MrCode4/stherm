import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
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
    StackView {
        anchors.fill: parent

        initialItem: Home {
            uiSession: mainView.uiSession
        }
    }
}

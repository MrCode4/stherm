import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * ApplicationMenu provides ui to access different section of application
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Get a referenct to UiSession
    property UiSession      uiSession

    /* Object properties
     * ****************************************************************************************/
    title: "Menu"
    contentItem: ApplicationMenuList {
        onMenuActivated: function(menuTitle) {
            //! Push related menu to stack
        }
    }
}

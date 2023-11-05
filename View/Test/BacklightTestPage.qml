import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * BacklightTestPage
 * ***********************************************************************************************/
BacklightPage {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    topPadding: 0
    bottomPadding: 32 * scaleFactor
    title: "Backlight Test"
    hasShades: false

    onUnshadedColorChanged: {
        //! Apply selected color to device immediately
    }

    Component.onCompleted: {
        //! Hide tick button and switch
        _root.header.contentItem.children[2].visible = false;
        //! Set switch to checked also
        _root.header.contentItem.children[2].children[0].checked = true;
    }
}

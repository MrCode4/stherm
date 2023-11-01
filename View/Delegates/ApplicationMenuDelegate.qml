import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ApplicationMenuDelegate is a simple delegate for ApplicationMenuList
 * ***********************************************************************************************/
ItemDelegate {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Holds delegate data
    property var    delegateData

    //! Holds delegate index
    property int    delegateIndex

    /* Object properties
     * ****************************************************************************************/
    contentItem: RowLayout {
        spacing: 4
        //! Icon: this is supposed to be a unicode of Font Awesome
        RoniaTextIcon {
            Layout.preferredWidth: implicitHeight * 1.5
            text: delegateData?.icon ?? ""
        }

        Label {
            Layout.fillWidth: true
            text: _root.text
            verticalAlignment: "AlignVCenter"
        }
    }
}

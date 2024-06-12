import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Icon of NEXGEN
 * \todo This should be designed as svg and displayed using Image
 * ***********************************************************************************************/
Item {
    /* Property declaration
 * ****************************************************************************************/
    //! Reference to I_Device
    property I_Device appModel

    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        source: appModel.contactContractor.iconSource
        sourceSize.width: parent.width
        cache: true
    }
}

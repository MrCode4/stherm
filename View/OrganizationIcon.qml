import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Icon of NEXGEN
 * \todo This should be designed as svg and displayed using Image
 * ***********************************************************************************************/
Image {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to I_Device
    property I_Device appModel

    /* Object properties
     * ****************************************************************************************/
    fillMode: Image.PreserveAspectFit
    source: appModel.contactContractor.iconSource
    sourceSize.width: width
    cache: true
}

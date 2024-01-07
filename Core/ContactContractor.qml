import QtQuick
import QtQuickStream

import Stherm

/*! ***********************************************************************************************
 * Contact contractor keeps the contractor properties.
 * ************************************************************************************************/
QSObject {
    id: root

    property string brandName :  "Nuve"

    property string phoneNumber: "(714) 471-7965"

    //! todo: use binary format
    property string iconSource:  "qrc:/Stherm/Images/nuve-icon.png"

    property string qrURL:       "https://www.nuvehome.com/"

    property string technicianURL: "https://upload.nuvehvac.com/#EN/USA/technician/view/"

}

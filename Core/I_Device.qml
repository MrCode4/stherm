import QtQuick
import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *
 * ************************************************************************************************/
QSObject {
    id: appModel

    /* Property Declarations
     * ****************************************************************************************/
    property real requestedTemp: 0.0

    property real currentTemp: 0.0

    property real requestedHum: 0.0

    property real currentHum: 0.0

    property Backlight backlight: Backlight {}

    /* Functions
     * ****************************************************************************************/
}

import QtQuick
import Qt.labs.settings

import Stherm

/*! ***********************************************************************************************
 * UiPreferences holds settings related to application ui
 * ***********************************************************************************************/
QtObject {
    id: _root

    /* Property Declarations
     * ****************************************************************************************/
    // Whether the window is fullscreen or windowed
    property int        windowMode:             Window.Windowed

}

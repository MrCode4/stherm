import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QTemplateProject

/*! ***********************************************************************************************
 * User Input Component
 * ************************************************************************************************/
RoniaUserInputBase {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property alias checked: switchControl.checked

    /* Object Properties
     * ****************************************************************************************/
    height: 35
    width: 200
    opacity: enabled ? 1 : 0.6

    /* Children
     * ****************************************************************************************/

    //! Switch
    editor: Switch {
        id: switchControl
    }
}

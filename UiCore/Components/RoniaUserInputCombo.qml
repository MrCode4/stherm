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
    property alias  currentIndex:   dirComboColorMode.currentIndex

    property var    values:         []

    property int    radius:         5

    readonly property var currentValue: values[currentIndex]    // for some reasons this is not working properly

    property alias isActive:        dirComboColorMode.isActive

    /* Object Properties
     * ****************************************************************************************/
    height: 35
    width: 200
    clip: true
    opacity: enabled ? 1 : 0.6

    /* Children
     * ****************************************************************************************/

    //! Number Editor Box
    editor: RoniaComboBox {
        id: dirComboColorMode
        fontSize: 7
        width: 110
        height: 35
        iconText: ""
        model: control.values
        radius: control.radius
    }

	//! Returns the associated index of value (values includes the value)
    function indexOfValue(value) {
        return dirComboColorMode.indexOfValue(value);
    }

}

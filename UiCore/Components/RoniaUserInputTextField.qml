import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QTemplateProject

/*! ***********************************************************************************************
 * User Input Text Field
 * ************************************************************************************************/
RoniaUserInputBase {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property alias  text:    textField.text

    /* Object Properties
     * ****************************************************************************************/
    height: 35
    width: 200
    opacity: enabled ? 1 : 0.6

    /* Children
     * ****************************************************************************************/
    //! TextField
    editor: TextField {
        id: textField
        font.family: "Roboto"
        font.pointSize: 10
        selectByMouse: true
    }
}

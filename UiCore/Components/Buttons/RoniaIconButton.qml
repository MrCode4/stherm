import QtQuick
import QtQuick.Controls
import QTemplateProject

/*! ***********************************************************************************************
 * The RIGIconButton is button with a icon as contentItem. The reason that the textIcon is not used
 * in this class is because it would overwrite the contentItem and style from the Button.qml class.
 * ************************************************************************************************/
RoniaBaseButton {
    id: rigIconButton

    /* Property Declarations
     * ****************************************************************************************/
    property int iconPixelSize: height * 0.8

    /* Object Properties
     * ****************************************************************************************/
    textColor: "white"

    contentItem: RoniaTextIcon {
        id: textIcon

        fontSizeMode: Text.Fit
        minimumPixelSize: 10
        font.pixelSize: iconPixelSize
        font.weight: 400

        color: !rigIconButton.enabled ? "gray" : textColor

        text: rigIconButton.text
    }
}

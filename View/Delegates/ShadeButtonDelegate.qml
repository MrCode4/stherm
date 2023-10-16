import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

/*! ***********************************************************************************************
 * ShadeButtonDelegate is a delegate for selecting shades of a color
 * ***********************************************************************************************/
RoundButton {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Source color of which a shade is provided
    property color          sourceColor

    //! Shade factor
    property real           shadeFactor

    //! Actual color of shaded
    readonly property alias shadeColor: _shadeColor.color

    /* Object properties
     * ****************************************************************************************/
    Layout.alignment: Qt.AlignCenter
    Layout.topMargin: checked ? -12 : 0
    background.implicitWidth: 52
    background.implicitHeight: 52
    checkable: true
    flat: !checked
    down: checked

    /* Children
     * ****************************************************************************************/
    Rectangle {
        id: _shadeColor
        anchors.centerIn: parent
        width: 32
        height: 32
        radius: width / 2
        border.width: 2
        border.color: _root.Material.foreground
        color: {
            return Qt.hsva(sourceColor.hsvHue,
                           sourceColor.hsvSaturation * shadeFactor,
                           sourceColor.hsvValue,
                           1.0);
        }
    }

    Behavior on Layout.topMargin { NumberAnimation { duration: 150 } }
}

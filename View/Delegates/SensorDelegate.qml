import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorDelegate is a delegate to represent a sensor
 * ***********************************************************************************************/
Button {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Sensor
    property Sensor     sensor

    //! Delegate index in model/view
    property int        delegateIndex

    /* Object properties
     * ****************************************************************************************/
    horizontalPadding: 16 * scaleFactor
    verticalPadding: 12 * scaleFactor
    text: sensor?.name ?? ""
    contentItem: RowLayout {
        Text {
            Layout.alignment: Qt.AlignCenter
            color: enabled ? _root.Material.foreground : _root.Material.hintTextColor
            font: _root.font
            text: _root.text
        }

        //! Some other infos about sensor, like signals, etc
    }
}

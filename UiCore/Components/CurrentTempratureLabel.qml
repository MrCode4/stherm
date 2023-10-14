import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * CurrentTempratureLabel shows current temprature in Fahrenheit or Celsius
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Holds current unit of temprature (Fahrenheit or Celsius)
    property string     unit: "F"

    //! \todo: add a property to get a ref to a temprature model
    //! I_Device
    property I_Device   device

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 8
    rightPadding: 8
    topPadding: 4
    bottomPadding: 4
    contentItem: GridLayout {
        columnSpacing: 0
        rowSpacing: 0
        columns: 2
        //! Current temprature
        Label {
            font {
                family: "Roboto Mono"
                pixelSize: 32
            }
            text: Number(device?.currentTemp ?? 0).toLocaleString(locale, "f", 0)
        }

        //! Unit
        Label {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 4
            opacity: 0.6
            font {
                pixelSize: 20
                capitalization: "AllUppercase"
            }
            text: `\u00b0${unit}`
        }

        //!
        Label {
            Layout.columnSpan: 2
            opacity: 0.6
            text: "Current"
        }
    }
}

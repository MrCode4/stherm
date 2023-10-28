import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * CurrentTempratureLabel shows current temprature in Fahrenheit or Celsius
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    property UiPreferences uiPreference

    //! Holds current unit of temprature (Fahrenheit or Celsius)
    property string     unit: (uiPreference?.tempratureUnit === UiPreferences.TempratureUnit.Fah ? "F" : "C") ?? "F"

    //! \todo: add a property to get a ref to a temprature model
    //! I_Device
    property I_Device   device

    /* Object properties
     * ****************************************************************************************/
    leftPadding: AppStyle.size / 60
    rightPadding: AppStyle.size / 60
    topPadding: AppStyle.size / 60 / 2
    bottomPadding: AppStyle.size / 60 / 2
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
            text: Number(uiPreference?.convertedTemperature(device?.currentTemp ?? 0) ?? 0).toLocaleString(locale, "f", 0)
        }

        //! Unit
        Label {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: AppStyle.size / 60 / 2
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

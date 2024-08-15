import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * TempratureLabel shows temprature in Fahrenheit or Celsius
 * Temperature default if current temperature
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Holds current unit of temprature (Fahrenheit or Celsius)
    property string     unit: (device?.setting?.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C") ?? "C"

    property real temperature: device?.currentTemp ?? 0

    property bool showCurrentTemperature: true

    //! \todo: add a property to get a ref to a temprature model
    //! I_Device
    property I_Device   device


    /* Object properties
     * ****************************************************************************************/
    leftPadding: 16 * scaleFactor
    rightPadding: 8 * scaleFactor
    topPadding: 10 * scaleFactor
    bottomPadding: 8 * scaleFactor
    contentItem: GridLayout {
        columnSpacing: 0
        rowSpacing: 0
        columns: 2
        //! Temprature
        Label {
            font {
                pointSize: Qt.application.font.pointSize * 1.5
            }
            text: Number(Utils.convertedTemperature(temperature, device?.setting?.tempratureUnit ?? AppSpec.TempratureUnit.Cel))
                        .toLocaleString(locale, "f", 0)
        }

        //! Unit
        Label {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: AppStyle.size / 60 / 2
            opacity: 0.6
            font {
                pointSize: Qt.application.font.pointSize * 1.2
                capitalization: "AllUppercase"
            }
            text: `\u00b0${unit}`
        }

        //! Current label
        Label {
            Layout.columnSpan: 2

            visible: showCurrentTemperature
            font.pointSize: Application.font.pointSize * 0.7
            opacity: 0.6
            text: "Current"
        }
    }
}

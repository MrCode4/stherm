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
    //! Holds current unit of temprature (Fahrenheit or Celsius)
    property string     unit: (device?.setting?.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C") ?? "F"

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
                pointSize: Qt.application.font.pointSize * 1.5
            }
            text: Number(Utils.convertedTemperature(device?.currentTemp ?? 0,
                                                      device?.setting?.tempratureUnit))
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

        //!
        Label {
            Layout.columnSpan: 2
            font.pointSize: Application.font.pointSize * 0.85
            opacity: 0.6
            text: "Current"
        }
    }
}

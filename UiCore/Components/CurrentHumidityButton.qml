import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * CurrentHumidityButton
 * ***********************************************************************************************/
ToolButton {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to an I_Device
    property I_Device   device

    /* Object properties
     * ****************************************************************************************/
    padding: 8

    /* Children
     * ****************************************************************************************/
    contentItem: RowLayout {
        spacing: 4

        //! Humidity icon
        Image {
            Layout.alignment: Qt.AlignCenter
            Layout.fillHeight: true
            Layout.preferredHeight: 0
            fillMode: Image.PreserveAspectFit
            source: "qrc:/Stherm/Images/water.png"
        }

        //! Humidity level label
        Label {
            Layout.alignment: Qt.AlignCenter
            font.pointSize: Qt.application.font.pointSize * 1.4
            text: Number(device?.currentHum ?? 0).toLocaleString(locale, "f", 0)
        }

        //! Percent label
        Label {
            Layout.alignment: Qt.AlignTop
            opacity: 0.65
            textFormat: "RichText"
            text: "<small>%</small>"
        }
    }
}

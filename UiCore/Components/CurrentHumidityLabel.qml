import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * HumidityLabel
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to an I_Device
    property I_Device   device

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 8
    rightPadding: 8
    topPadding: 4
    bottomPadding: 4
    background: null

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
            font.pixelSize: 24
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

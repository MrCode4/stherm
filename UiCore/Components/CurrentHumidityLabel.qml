import QtQuick
import QtQuick.Layouts

import Ronia
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
    leftPadding: AppStyle.size / 60
    rightPadding: AppStyle.size / 60
    topPadding: AppStyle.size / 60 / 2
    bottomPadding: AppStyle.size / 60 / 2
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
            font.pixelSize: AppStyle.size / 20
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

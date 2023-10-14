import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * DesiredTempratureItem provides a ui for setting desired temprature
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Unit of temprature
    property string     unit: "F"

    //! Minimum temprature
    property int        minTemprature: 0

    //! Maximum temprature
    property int        maxTemprature: 100

    /* Object properties
     * ****************************************************************************************/
    //    Material.theme: Material.Dark
    implicitWidth: 360
    implicitHeight: 180
    background: null
    contentItem: Item {
        SemiCircleSlider {
            id: _tempSlider
            anchors.fill: parent
            from: minTemprature
            to: maxTemprature
        }

        //! Desired temprature label
        Label {
            id: _desiredTempratureLbl
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -8
            font.pixelSize: 48
            text: Number(_tempSlider.value).toLocaleString(locale, "f", 0)

            //! Unit
            Label {
                anchors.left: parent.right
                anchors.top: parent.top
                anchors.topMargin: 20
                opacity: 0.6
                font {
                    pixelSize: 20
                    capitalization: "AllUppercase"
                }
                text: `\u00b0${unit}`
            }
        }
    }
}

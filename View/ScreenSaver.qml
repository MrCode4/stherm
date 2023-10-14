import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * ScreenSaver Item
 * ***********************************************************************************************/
Popup {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to I_Device
    property I_Device   device

    //! Unit
    property string     unit:       "F"

    /* Object properties
     * ****************************************************************************************/
    implicitHeight: 480
    implicitWidth:  480
    background: Rectangle {
        color: _root.Material.background
    }
    closePolicy: Popup.NoAutoClose

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.centerIn: parent

        //! Temprature Label
        Label {
            id: _tempratureLbl

            Layout.alignment: Qt.AlignCenter
            font.pointSize: 80
            text: Number(device?.currentTemp ?? 0).toLocaleString(locale, "f", 0)

            Label {
                anchors {
                    left: parent.right
                    top: parent.top
                    topMargin: 20
                }

                opacity: 0.6
                font {
                    pixelSize: 40
                    capitalization: "AllUppercase"
                }
                text: `\u00b0${unit}`
            }
        }

        //! Mode button
        ToolButton {
            //! Set icon.source: according to mode
        }

        //! NEXGEN icon
        NexgenIcon {
            Layout.alignment: Qt.AlignHCenter
        }
    }
}

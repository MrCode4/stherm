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
    implicitHeight: AppStyle.size
    implicitWidth:  AppStyle.size
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
            font.pointSize: AppStyle.size / 6
            text: Number(device?.currentTemp ?? 0).toLocaleString(locale, "f", 0)

            Label {
                anchors {
                    left: parent.right
                    top: parent.top
                    topMargin: AppStyle.size / 24
                }

                opacity: 0.6
                font {
                    pixelSize: AppStyle.size / 12
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

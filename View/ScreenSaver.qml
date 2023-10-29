import QtQuick
import QtQuick.Layouts

import Ronia
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

    property UiPreferences uiPreference

    //! Unit
    property string     unit: uiPreference?.tempratureUnit === UiPreferences.TempratureUnit.Fah ? "F" : "C" ?? "F"

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
            text: Number(uiPreference?.convertedTemperature(device?.currentTemp ?? 0) ?? 0).toLocaleString(locale, "f", 0)

            Label {
                anchors {
                    left: parent.right
                    top: parent.top
                    topMargin: AppStyle.size / 24
                }

                opacity: 0.6
                font {
                    pointSize: Qt.application.font.pointSize * 2.4
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
            Layout.preferredWidth: _root.width * 0.75
            Layout.preferredHeight: sourceSize.height * width / sourceSize.width
        }
    }
}

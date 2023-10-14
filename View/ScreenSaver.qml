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
    //! Temprature: holds current temprature. This will probably be replaced by a ref to a model and
    //! reading temprature from that model
    property int        temprature: 72

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
            text: temprature
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

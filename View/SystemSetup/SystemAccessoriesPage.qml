import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemAccessoriesPage provides a ui for controlling system accesories
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Accessories"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            //! Apply settings and pop this from StackView
            //! Apply settings here

            //! Also move out of this Page
            backButtonCallback();
        }
    }

    GridLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9
        columns: 3
        columnSpacing: 4
        rowSpacing: 4

        Label {
            Layout.columnSpan: 3
            text: "Humidifier"
        }

        //! Humidifier CheckBox 1
        CheckBox {
            Layout.leftMargin: 40 * scaleFactor
            checked: true
            text: "T1\npwrd"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Humidifier CheckBox 2
        CheckBox {
            text: "T1\nshort"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Humidifier CheckBox 3
        CheckBox {
            text: "T2\npwrd"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        Label {
            Layout.columnSpan: 3
            Layout.topMargin: 40
            text: "Dehumidifier"
        }

        //! Dehumidifier CheckBox 1
        CheckBox {
            Layout.leftMargin: 40 * scaleFactor
            checked: true
            text: "T1\npwrd"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Dehumidifier CheckBox 2
        CheckBox {
            text: "T1\nshort"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Dehumidifier CheckBox 3
        CheckBox {
            text: "T2\npwrd"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }
    }
}

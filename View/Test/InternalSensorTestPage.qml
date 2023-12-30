import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InternalSensorTestPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    title: "Internal Sensor Test"

    /* Children
     * ****************************************************************************************/
    //! Next button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            //! Next page
            if (_root.StackView.view) {
                _root.StackView.view.push("qrc:/Stherm/View/Test/RelayTestPage.qml", {
                                              "uiSession": uiSession
                                          })
            }
        }
    }

    GridLayout {
        anchors.fill: parent
        columns: 2

        Label {
            text: "Temprature :"
        }

        Item { }

        Label {
            text: "Humidity :"
        }

        Item { }

        Label {
            text: "TOF :"
        }

        Item { }

        Label {
            text: "Ambient :"
        }

        Item { }

        Label {
            text: "CO2 :"
        }

        Item { }
    }
}

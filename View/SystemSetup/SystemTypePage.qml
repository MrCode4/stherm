import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypePage provides ui for choosing system type in SystemSetupPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Type"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.5
        spacing: 12

        Button {
            Layout.fillWidth: true
            text: "Traditional"

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_traditionalPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Heat Pump"

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_heatPumpPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Cool Only"

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_coolonlyPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Heat Only"

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_heatonlyPageCompo);
                }
            }
        }
    }


    //! System type page components
    Component {
        id: _traditionalPageCompo

        SystemTypeTraditionPage {
            uiSession: root.uiSession
        }
    }

    Component {
        id: _heatPumpPageCompo

        SystemTypeHeatPumpPage {
            uiSession: root.uiSession
        }
    }

    Component {
        id: _coolonlyPageCompo

        SystemTypeCoolOnlyPage {
            uiSession: root.uiSession
        }
    }

    Component {
        id: _heatonlyPageCompo

        SystemTypeHeatOnlyPage {
            uiSession: root.uiSession
        }
    }
}

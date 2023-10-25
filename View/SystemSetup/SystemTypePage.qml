import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypePage provides ui for choosing system type in SystemSetupPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Type"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 8

        Button {
            Layout.fillWidth: true
            text: "Traditional"

            onClicked: {
                //! Move to corresponding page
                if (_root.StackView.view) {
                    _root.StackView.view.push(_traditionalPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Heat Pump"

            onClicked: {
                //! Move to corresponding page
                if (_root.StackView.view) {
                    _root.StackView.view.push(_heatPumpPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Cool Only"

            onClicked: {
                //! Move to corresponding page
                if (_root.StackView.view) {
                    _root.StackView.view.push(_coolonlyPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Heat Only"

            onClicked: {
                //! Move to corresponding page
                if (_root.StackView.view) {
                    _root.StackView.view.push(_heatonlyPageCompo);
                }
            }
        }
    }


    //! System type page components
    Component {
        id: _traditionalPageCompo

        SystemTypeTraditionPage { }
    }

    Component {
        id: _heatPumpPageCompo

        SystemTypeHeatPumpPage { }
    }

    Component {
        id: _coolonlyPageCompo

        SystemTypeCoolOnlyPage { }
    }

    Component {
        id: _heatonlyPageCompo

        SystemTypeHeatOnlyPage { }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemSetupPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Setup"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 8

        Button {
            Layout.fillWidth: true
            text: "System Type"

            onClicked: {
                //! Show corresponding page
                if (_root.StackView.view) {
                    _root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
                                                  "uiSession": uiSession
                                              });
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "System Stages"

            onClicked: {
                //! Show corresponding page
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Accessories"

            onClicked: {
                //! Show corresponding page
                if (_root.StackView.view) {
                    _root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemAccessoriesPage.qml", {
                                                  "uiSession": uiSession
                                              });
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "System Run Delay"

            onClicked: {
                //! Show corresponding page
                if (_root.StackView.view) {
                    _root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemRunDelayPage.qml", {
                                                  "uiSession": uiSession
                                              });
                }
            }
        }
    }
}

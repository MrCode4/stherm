import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemSetupPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup : false

    /* Object properties
     * ****************************************************************************************/
    title: "System Setup"

    Component.onCompleted: {
        deviceController.updateLockMode(AppSpec.EMSystemSetup, true);
    }

    Component.onDestruction: {
        deviceController.updateEditMode(AppSpec.EMSystemSetup);
        deviceController.saveSettings();

        deviceController.updateLockMode(AppSpec.EMSystemSetup, false);
    }

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.9, implicitWidth)
        spacing: 12

        Button {
            Layout.fillWidth: true
            text: "System Type"

            onClicked: {
                //! Show corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
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
                switch(appModel.systemSetup.systemType) {
                    case AppSpec.Conventional:
                        root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypeTraditionPage.qml", {
                                                      "uiSession": root.uiSession
                                                  });
                        break;
                    case AppSpec.HeatPump:
                        root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypeHeatPumpPage.qml", {
                                                      "uiSession": root.uiSession
                                                  });
                        break;
                    case AppSpec.CoolingOnly:
                        root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypeCoolOnlyPage.qml", {
                                                      "uiSession": root.uiSession
                                                  });
                        break;
                    case AppSpec.HeatingOnly:
                        root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypeHeatOnlyPage.qml", {
                                                      "uiSession": root.uiSession
                                                  });
                        break;

                    case AppSpec.DualFuelHeating:
                        root.StackView.view.push("qrc:/Stherm/View/SystemSetup/DualFuelHeatingPage.qml", {
                                                     "uiSession": root.uiSession
                                                 });
                        break;
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Accessories"

            onClicked: {
                //! Show corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemAccessoriesPage.qml", {
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
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemRunDelayPage.qml", {
                                                  "uiSession": uiSession
                                              });
                }
            }
        }
    }
}

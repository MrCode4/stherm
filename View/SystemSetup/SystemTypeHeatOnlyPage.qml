import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeHeatOnlyPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "Heat Only"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: initialSetup ? FAIcons.arrowRight : "\uf00c"
        }

        onClicked: {
            //! Do neccessary updates
            if (deviceController) {
                deviceController.setSystemHeatOnly(heatStageLayout.heatStage)
            }

            if (initialSetup) {
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemAccessoriesPage.qml", {
                                                 "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup
                                             });
                }
            } else {
                //! Also move out of this Page
                goToSystemTypePage();
            }
        }
    }

    RowLayout {
        id: heatStageLayout

        property int heatStage: 1

        anchors.centerIn: parent
        spacing: 48

        Label {
            Layout.fillWidth: true
            text: "Heat Stages"
        }

        RowLayout {
            Layout.fillWidth: false

            RadioButton {
                checked: appModel.systemSetup.heatStage === Number(text)
                onCheckedChanged:  {
                    if (checked)
                        heatStageLayout.heatStage = Number(text);
                }

                text: "1"
            }

            RadioButton {
                checked: appModel.systemSetup.heatStage === Number(text)
                onCheckedChanged: {
                    if (checked)
                        heatStageLayout.heatStage = Number(text);
                }

                text: "2"
            }

            RadioButton {
                checked: appModel.systemSetup.heatStage === Number(text)
                onCheckedChanged: {
                    if (checked)
                        heatStageLayout.heatStage = Number(text);
                }

                text: "3"
            }
        }
    }

    function goToSystemTypePage()
    {
        if (initialSetup) {
            backButtonCallback();
            return;
        }

        if (root.StackView.view) {
            //! Then Page is inside an StackView
            if (root.StackView.view.currentItem == root) {
                //! Pop twice to get back to SystemSetupPage
                if (root.StackView.view.get(root.StackView.view.depth - 2, StackView.DontLoad) instanceof SystemTypePage) {
                    root.StackView.view.pop();
                }
                root.StackView.view.pop();
            }
        }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeCoolOnlyPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "Cool Only"

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
                deviceController.setSystemCoolingOnly(coolStageLayout.coolStage)
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
        id: coolStageLayout

        property int coolStage: 1

        anchors.centerIn: parent
        spacing: 48

        Label {
            Layout.fillWidth: true
            text: "Cool Stages"
        }

        RowLayout {
            Layout.fillWidth: false

            RadioButton {
                checked: appModel.systemSetup.coolStage === Number(text)
                onCheckedChanged: {
                    if (checked)
                        coolStageLayout.coolStage = Number(text);
                }

                text: "1"
            }

            RadioButton {
                checked: appModel.systemSetup.coolStage === Number(text)
                onCheckedChanged: {
                    if (checked)
                        coolStageLayout.coolStage = Number(text);
                }

                text: "2"
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

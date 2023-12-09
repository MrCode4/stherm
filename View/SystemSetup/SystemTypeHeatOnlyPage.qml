import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeHeatOnlyPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Heat Only"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Do neccessary updates
            if (deviceController) {
                deviceController.setSystemHeatOnly(heatStageLayout.heatStage)
            }

            //! Also move out of this Page
            backButtonCallback();
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
}

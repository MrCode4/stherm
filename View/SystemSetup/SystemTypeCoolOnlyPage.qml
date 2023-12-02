import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeCoolOnlyPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Cool Only"

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

            appModel.systemSetup.systemType = 2; // 2: CoolingOnly
            appModel.systemSetup.coolStage  = coolStageLayout.coolStage;

            //! Also move out of this Page
            backButtonCallback();
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
}

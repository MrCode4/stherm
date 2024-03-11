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
    property bool initialSetup: false

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
            if (deviceController) {
                deviceController.setSystemCoolingOnly(coolStageLayout.coolStage)
            }

            //! Also move out of this Page
            goToSystemTypePage();
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

        if (_root.StackView.view) {
            //! Then Page is inside an StackView
            if (_root.StackView.view.currentItem == _root) {
                //! Pop twice to get back to SystemSetupPage
                if (_root.StackView.view.get(_root.StackView.view.depth - 2, StackView.DontLoad) instanceof SystemTypePage) {
                    _root.StackView.view.pop();
                }
                _root.StackView.view.pop();
            }
        }
    }
}

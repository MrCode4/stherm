import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemRunDelayPage provides ui for selecting run delay of system
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property int systemRunDelay: 1

    property bool initialSetup : false

    /* Object properties
     * ****************************************************************************************/
    title: "System Run Delay"

    /* Children
     * ****************************************************************************************/


    //! Confirm button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        visible: !initialSetup
        enabled: !initialSetup

        onClicked: {
            updateModel()

            if (root.StackView.view) {
                //! pop this from StackView
                root.StackView.view.pop()
            }
        }
    }

    //! Next button in initial setup flow
    ButtonInverted {
        text: "Next"

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        visible: initialSetup
        enabled: appModel?.serviceTitan?._fetched ?? false
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            updateModel();
            nextPage();
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.alignment: Qt.AlignCenter
            textFormat: "MarkdownText"
            text: "### Time"
        }

        RowLayout {
            RadioButton {
                text: "1 min"
                checked: appModel.systemSetup.systemRunDelay === 1

                onCheckedChanged: {
                    if (checked)
                        systemRunDelay = 1;
                }
            }
            RadioButton {
                text: "2 min"
                checked: appModel.systemSetup.systemRunDelay === 2

                onCheckedChanged: {
                    if (checked)
                        systemRunDelay = 2;
                }
            }
            RadioButton {
                text: "5 min"
                checked: appModel.systemSetup.systemRunDelay === 5

                onCheckedChanged: {
                    if (checked)
                        systemRunDelay = 5;
                }
            }
        }
    }

    function updateModel() {
        //! Apply settings
        if (deviceController) {
            deviceController.setSystemRunDelay(root.systemRunDelay)
        }
    }

    function nextPage() {
        if (root.StackView.view) {
            if (appModel?.serviceTitan?.isActive ?? false) {
                root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/JobNumberPage.qml", {
                                             "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                         });
            } else {
                // Go to TechnicianDetailsPage
                root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/TechnicianDetailsPage.qml", {
                                             "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                         });
            }

        }
    }
}

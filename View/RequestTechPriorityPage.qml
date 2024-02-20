import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * RequestTechPriority page
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    title: "Request a Tech Priority"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton   {
        id: confirmtBtn
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            // Update model and send request tech

        }
    }

    ButtonGroup {
        buttons: buttonsLay.children
    }

    ColumnLayout {
        id: buttonsLay
        anchors.centerIn: parent
        width: parent.width * 0.5
        spacing: 12

        Button {
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: true
            text: "Urgent"

            onClicked: {
            }
        }

        Button {
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            text: "Regular"

            onClicked: {
            }
        }
    }

}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InstallationTypePage provides ui for choosing installation type.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    title: "Installation Type"

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.5
        spacing: 12

        Button {
            Layout.fillWidth: true
            text: "New installation"
            autoExclusive: true

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push();
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Existing system"
            autoExclusive: true

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push();
                }
            }
        }
    }
}

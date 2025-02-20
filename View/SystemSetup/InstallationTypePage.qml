import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InstallationTypePage provides ui for choosing installation type.
 * ***********************************************************************************************/
InitialSetupBasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    title: "Installation Type"

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.65
        spacing: 12

        Button {
            Layout.fillWidth: true
            text: "New installation"
            autoExclusive: true

            onClicked: {
                appModel.installationType = AppSpec.InstallationType.ITNewInstallation;
                nextPage();
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Existing system"
            autoExclusive: true

            onClicked: {
                appModel.installationType = AppSpec.InstallationType.ITExistingSystem;
                nextPage();
            }
        }
    }

    /* Functions
     * ****************************************************************************************/
    function nextPage() {

        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/SystemSetup/ResidenceTypePage.qml", {
                                          "uiSession": uiSession,
                                         "initialSetup": root.initialSetup
                                      });
        }
    }
}

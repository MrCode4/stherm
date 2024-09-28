import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InstallationTypePage provides ui for choosing installation type.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false


    /* Object properties
     * ****************************************************************************************/
    title: "Installation Type"

    /* Children
     * ****************************************************************************************/
    //! Info button in initial setup mode.
    InfoToolButton {
        parent: root.header.contentItem
        visible: initialSetup

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                             "uiSession": Qt.binding(() => uiSession)
                                         });
            }

        }
    }

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

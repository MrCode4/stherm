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
    ToolButton {
        parent: root.header.contentItem
        checkable: false
        checked: false
        visible: initialSetup
        implicitWidth: 64
        implicitHeight: implicitWidth
        icon.width: 50
        icon.height: 50

        contentItem: RoniaTextIcon {
            anchors.fill: parent
            font.pointSize: Style.fontIconSize.largePt
            Layout.alignment: Qt.AlignLeft
            text: FAIcons.circleInfo
        }

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
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/ResidenceTypePage.qml", {
                                                  "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup
                                              });
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Existing system"
            autoExclusive: true

            onClicked: {
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/ResidenceTypePage.qml", {
                                                  "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup
                                              });
                }
            }
        }
    }
}

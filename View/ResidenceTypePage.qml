import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ResidenceTypePage provides ui for choosing residence type and device location
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "Residence type"

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
        width: parent.width * 0.5
        spacing: 12

        Button {
            Layout.fillWidth: true
            text: "House"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.Conventional

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push();
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Apartment"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.Conventional

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push();
                }
            }
        }
        Button {
            Layout.fillWidth: true
            text: "Commercial"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.Conventional

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push();
                }
            }
        }
        Button {
            Layout.fillWidth: true
            text: "Office"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.Conventional

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push();
                }
            }
        }
        Button {
            Layout.fillWidth: true
            text: "Industrial"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.Conventional

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push();
                }
            }
        }
        Button {
            Layout.fillWidth: true
            text: "Garage"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.Conventional

            onClicked: {
            }
        }
    }
}

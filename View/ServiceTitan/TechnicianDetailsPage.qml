import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ThermostatNamePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false


    /* Object properties
     * ****************************************************************************************/
    title: "Technician Details"

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
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        spacing: 4

        Label {
            text: "Full name"
            font.pointSize: root.font.pointSize * 1.1
        }

        TextField {
            id: nameTf

            Layout.fillWidth: true
            placeholderText: "Input Technician’s Full name"
            text: appModel?.serviceTitan?.fullName ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
            }

            onAccepted: {
                nextBtn.forceActiveFocus();
                nextBtn.clicked();
            }
        }

        Item {
            Layout.fillWidth: true
            height: root.height / 6
        }

        Text {
            id: warrantyReplacementText

            text: qsTr("Warranty Replacement")
            font.underline: true
            color: "#43E0F8"

            TapHandler {
                onTapped: {
                    // TODO: Go to warranty page

                }
            }
        }
    }

    //! Next button
    ButtonInverted {
        id: nextBtn

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        text: "Next"
        visible: !nameTf.activeFocus
        enabled: nameTf.text.length > 0
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            appModel.serviceTitan.fullName = nameTf.text;

            // Go to next page
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/CustomerDetailsPage.qml", {
                                             "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                         });
            }
        }
    }
}

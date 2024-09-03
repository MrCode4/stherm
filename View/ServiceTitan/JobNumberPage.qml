import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * JobNumberPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "Job Number"

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

    GridLayout {
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        rowSpacing: 20
        columnSpacing: 8
        columns: 2

        Label {
            Layout.columnSpan: 2
            text: "Job number"
            font.pointSize: root.font.pointSize * 1.1
        }

        TextField {
            id: jobNumberTf

            Layout.preferredWidth: parent.width * 0.8

            placeholderText: "Input the job number"
            text: appModel?.serviceTitan?.jobNumber ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
            }

            onAccepted: {
            }
        }

        Text {
            id: skipCheckText

            Layout.alignment: Qt.AlignVCenter

            text: jobNumberTf.text.length > 0 ? qsTr("Check") : qsTr("Skip")
            color: "#43E0F8"

            TapHandler {
                onTapped: {
                    appModel.serviceTitan.jobNumber = jobNumberTf.text;

                    if (jobNumberTf.text.length > 0) {
                        appModel.serviceTitan.isSTManualMode = false;
                        // TODO: Check the job number
                        if (root.StackView.view) {
                            root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/CustomerDetailsPage.qml", {
                                                         "uiSession": uiSession,
                                                         "initialSetup": root.initialSetup
                                                     });
                        }
                    } else {
                        // Skip
                        appModel.serviceTitan.isSTManualMode = true;
                        if (root.StackView.view) {
                            root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/TechnicianDetailsPage.qml", {
                                                         "uiSession": uiSession,
                                                         "initialSetup": root.initialSetup
                                                     });
                        }
                    }
                }
            }
        }

        Label {
            width: jobNumberTf.width

            Layout.columnSpan: 2
            Layout.preferredWidth: parent.width * 0.8

            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.7
            text: "Enter the ServiceTitan Job number and click \"Check\" to auto-fill the fields." +
                  "If you don't have one, click \"Skip.\""
        }

        Text {
            id: warrantyReplacementText

            Layout.columnSpan: 2
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
}

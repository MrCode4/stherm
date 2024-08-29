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
                                             "uiSession": Qt.binding(() => uiSession),
                                             "initialSetup": root.initialSetup
                                         });
            }

        }
    }

    //! Next button in initial setup flow
    ButtonInverted {
        text: "Next"

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        visible: initialSetup && appModel.residenceType === AppSpec.ResidenceTypes.Garage;
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            // if (root.StackView.view) {
            //     root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemAccessoriesPage.qml", {
            //                                   "uiSession": uiSession,
            //                                  "initialSetup": root.initialSetup
            //                               });
            // }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.5
        spacing: 12

        Repeater {
            model: Object.keys(AppSpec.residenceTypesNames)
            delegate: Button {
                Layout.fillWidth: true
                text: AppSpec.residenceTypesNames[modelData]
                autoExclusive: true
                checked: appModel?.residenceType === modelData

                onClicked: {
                    appModel.residenceType = modelData;

                    if (appModel.residenceType !== AppSpec.ResidenceTypes.Garage) {
                        console.log("MAOd", modelData)
                        if (root.StackView.view) {
                            root.StackView.view.push("qrc:/Stherm/View/DeviceLocationPage.qml", {
                                                         "uiSession": Qt.binding(() => uiSession)
                                                     });
                        }
                    }
                }
            }
        }
    }
}

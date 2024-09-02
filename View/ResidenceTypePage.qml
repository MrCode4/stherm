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

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.5
        spacing: 12

        Repeater {
            model: Object.keys(AppSpec.residenceTypesNames)
            delegate: Button {
                //! Convert model data to number
                property int modelDataNum: Number(modelData)

                Layout.fillWidth: true
                text: AppSpec.residenceTypesNames[modelDataNum]
                autoExclusive: true
                checked: appModel?.residenceType === modelDataNum

                onClicked: {
                    if (appModel.residenceType !== modelDataNum) {
                        appModel.residenceType = modelDataNum;
                        appModel.deviceLocation = ""
                    }

                    nextPageTimer.goNextPage = true;
                }
            }
        }
    }

    Timer {
        id: nextPageTimer

        property bool goNextPage: false

        interval: 5000
        repeat: false
        running: root.visible && goNextPage &&
                 appModel.residenceType !== AppSpec.ResidenceTypes.Unknown

        onTriggered: {
            goNextPage = false;

            if (appModel.residenceType === AppSpec.ResidenceTypes.Garage) {
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/ThermostatNamePage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession),
                                                 "initialSetup":  root.initialSetup
                                             });
                }

            } else {
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/DeviceLocationPage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession),
                                                 "initialSetup":  root.initialSetup
                                             });
                }
            }
        }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * DeviceLocationPage provides ui for choosing device location
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    property string deviceLocation: appModel?.deviceLocation ?? ""

    /* Object properties
     * ****************************************************************************************/
    title: "Device Location"

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

    Flickable {
        id: itemsFlickable

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 16
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: _contentLay.implicitHeight
        contentWidth: width

        ColumnLayout {
            id: _contentLay

            anchors.centerIn: parent
            width: parent.width * 0.65

            Repeater {
                model: AppSpec.deviceLoacations[appModel?.residenceType ?? AppSpec.Unknown]

                delegate: Button {
                    Layout.fillWidth: true
                    text: modelData
                    autoExclusive: true
                    checked: deviceLocation === text

                    onClicked: {
                        appModel.deviceLocation = String(modelData);
                        nextPage();
                    }
                }
            }
        }
    }

    /* Functions
     * ****************************************************************************************/

    function nextPage() {
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/SystemSetup/ThermostatNamePage.qml", {
                                         "uiSession": Qt.binding(() => uiSession),
                                         "initialSetup":  root.initialSetup
                                     });
        }
    }
}

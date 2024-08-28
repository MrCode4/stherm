import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypePage provides ui for choosing system type in SystemSetupPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "System Type"

    backButtonCallback: function() {
        if (root.StackView.view) {
            //! Then Page is inside an StackView
            if (root.StackView.view.currentItem === root) {
                root.StackView.view.pop();

                if (root.StackView.view.currentItem instanceof PrivacyPolicyPage) {
                    root.StackView.view.pop();
                }
            }
        }
    }

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
            text: "Traditional"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.Conventional

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_traditionalPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Heat Pump"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.HeatPump

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_heatPumpPageCompo);
                }
            }
        }

        //! Dual Fuel Heating
        Button {
            Layout.fillWidth: true
            text: "Dual Fuel Heating"
            autoExclusive: true
            enabled: false
            // checked: appModel.systemSetup.systemType === AppSpec.CoolingOnly

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    // root.StackView.view.push(_coolonlyPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Cool Only"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.CoolingOnly

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_coolonlyPageCompo);
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Heat Only"
            autoExclusive: true
            checked: appModel.systemSetup.systemType === AppSpec.HeatingOnly

            onClicked: {
                //! Move to corresponding page
                if (root.StackView.view) {
                    root.StackView.view.push(_heatonlyPageCompo);
                }
            }
        }
    }


    //! System type page components
    Component {
        id: _traditionalPageCompo

        SystemTypeTraditionPage {
            uiSession: root.uiSession
            initialSetup: root.initialSetup

        }
    }

    Component {
        id: _heatPumpPageCompo

        SystemTypeHeatPumpPage {
            uiSession: root.uiSession
            initialSetup: root.initialSetup
        }
    }

    Component {
        id: _coolonlyPageCompo

        SystemTypeCoolOnlyPage {
            uiSession: root.uiSession
            initialSetup: root.initialSetup
        }
    }

    Component {
        id: _heatonlyPageCompo

        SystemTypeHeatOnlyPage {
            uiSession: root.uiSession
            initialSetup: root.initialSetup
        }
    }
}

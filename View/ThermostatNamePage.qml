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
    implicitHeight: implicitHeaderHeight * 6 + _nameTf.implicitHeight + topPadding + bottomPadding
    title: "Thermostat Name"

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

    TextField {
        id: _nameTf
        anchors.horizontalCenter: parent.horizontalCenter
        y: height * 0.8
        width: parent.width * 0.7
        placeholderText: "Input the Name"
        text: appModel?.thermostatName ?? ""
        validator: RegularExpressionValidator {
            regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
        }

        onAccepted: {
            if (submitBtn.visible) {
                submitBtn.forceActiveFocus();
                submitBtn.clicked();
            }

        }
    }

    //! Submit button
    ButtonInverted {
        id: submitBtn

        text: "Submit"

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        leftPadding: 25
        rightPadding: 25

        onClicked: {
            appModel.thermostatName = _nameTf.text;
        }
    }
}

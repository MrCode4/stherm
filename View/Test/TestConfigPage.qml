import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * TestConfigPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property DeviceAPI deviceAPI: deviceController.deviceControllerCPP.deviceAPI

    property var testConfigs: deviceAPI.testConfigs();

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 8 * scaleFactor
    rightPadding: 12 * scaleFactor
    title: "Test Configurations"

    backButtonCallback: function() {
        var tc = deviceAPI.testConfigs();
        //! Check if domain is modified
        if (tc.testConfigIp === ipTF.text &&
            tc.testConfigUser === userTF.text &&
            tc.testConfigPassword === passwordTF.text &&
            tc.testConfigDestination === destinationTF.text) {

            tryGoBack()

        } else {
            //! This means that changes are occured that are not saved into model
            uiSession.popUps.exitConfirmPopup.accepted.connect(confirmtBtn.clicked);
            uiSession.popUps.exitConfirmPopup.rejected.connect(tryGoBack);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.exitConfirmPopup);
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        id: confirmtBtn
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! Save
            deviceAPI.setTestConfigs(ipTF.text, userTF.text, passwordTF.text, destinationTF.text);

            tryGoBack()
        }
    }

    GridLayout {
        id: _contentLay
        width: parent.width
        columns: 2
        columnSpacing: 8 * scaleFactor
        rowSpacing: 2 * scaleFactor

        Label {
            text: "ip:"
            font.pointSize: root.font.pointSize * 0.9
        }

        TextField {
            id: ipTF

            Layout.fillWidth: true
            placeholderText: "ip"
            text: testConfigs?.testConfigIp ?? ""
            inputMethodHints: Qt.ImhPreferNumbers
            font.pointSize: root.font.pointSize * 0.8

        }

        Label {
            text: "Username:"
            font.pointSize: root.font.pointSize * 0.9
        }

        TextField {
            id: userTF

            Layout.fillWidth: true
            placeholderText: "Username"
            text: testConfigs?.testConfigUser ?? ""
            font.pointSize: root.font.pointSize * 0.8
        }

        Label {
            text: "Password:"
            font.pointSize: root.font.pointSize * 0.9
        }

        TextField {
            id: passwordTF

            Layout.fillWidth: true
            rightPadding: _passwordEchoBtn.width
            placeholderText: "Password"
            text: testConfigs?.testConfigPassword ?? ""
            font.pointSize: root.font.pointSize * 0.8
            echoMode: _passwordEchoBtn.checked ? TextField.Normal : TextField.Password

            ToolButton {
                id: _passwordEchoBtn
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    verticalCenterOffset: -6
                }
                checkable: true
                focusPolicy: "NoFocus"
                contentItem: RoniaTextIcon {
                    font.pointSize: Style.fontIconSize.smallPt
                    text: _passwordEchoBtn.checked ? FAIcons.eyeSlash : FAIcons.eye
                }
            }
        }

        Label {
            text: "Destination:"
            font.pointSize: root.font.pointSize * 0.9
        }

        TextField {
            id: destinationTF

            Layout.fillWidth: true
            placeholderText: "Destination"
            text: testConfigs?.testConfigDestination ?? ""
            font.pointSize: root.font.pointSize * 0.8
        }

        Item {
            Layout.columnSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

    }

    //! Reset setting Button
    ButtonInverted {
        id: resetButton
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10
        text: "Clear"

        onClicked: {
            testConfigs = {};
        }
    }
}

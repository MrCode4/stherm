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

    property System system:     deviceController?.deviceControllerCPP?.system ?? null

    /* Object properties
     * ****************************************************************************************/
    title: "Warranty Replacement"

    /* Children
     * ****************************************************************************************/
    Label {
        id: warranryReplacementInfo

        anchors.top: parent.top
        anchors.topMargin: 10

        width: parent.width * 0.95
        wrapMode: Text.WordWrap
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: root.font.pointSize * 0.67
        text: "To replace the damaged thermostat under warranty, " +
              "enter the serial number (S/N) of the damaged thermostat " +
              "in the 'Old S/N' box below. Then click 'Replace' to proceed."
    }

    ColumnLayout {
        anchors.top: warranryReplacementInfo.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.9

        Label {
            text: "Old S/N"
            font.pointSize: root.font.pointSize
        }

        TextField {
            id: oldSNTf

            topPadding: 0
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            placeholderText: "Input the S/N of damaged thermostat"
            validator: RegularExpressionValidator {
                regularExpression: /^\d{2}-\d{3}-\d{6}$/
            }
        }

        Label {
            Layout.fillWidth: true

            width: parent.width
            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.67
            text: "Check the S/N (Serial Number) on the back of the thermostat (e.g., 01224001212)."
        }

        Item {
            id: spacer
            Layout.fillWidth: true
            height: 10
        }

        Label {
            text: "New S/N"
            font.pointSize: root.font.pointSize
        }

        TextField {
            id: newSNTf

            Layout.fillWidth: true

            // Device with the new serial number
            text: system?.serialNumber ?? ""
            topPadding: 0
            Layout.preferredHeight: 50
            placeholderText: "New S/N"
            validator: RegularExpressionValidator {
                regularExpression: /^\d{2}-\d{3}-\d{6}$/
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    //! Replace button
    ButtonInverted {
        id: replaceBtn

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        text: "Replace"
        visible: !oldSNTf.activeFocus && !newSNTf.activeFocus
        enabled: oldSNTf.acceptableInput && newSNTf.acceptableInput
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            system?.warrantyReplacement(oldSNTf.text, newSNTf.text);
        }
    }

}

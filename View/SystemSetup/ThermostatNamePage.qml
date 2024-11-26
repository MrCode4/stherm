import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ThermostatNamePage
 * ***********************************************************************************************/
InitialSetupBasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Busy due to get the Install operation
    property bool isBusy: deviceController.isSendingInitialSetupData

    /* Object properties
     * ****************************************************************************************/
    title: "Thermostat Name"

    showWifiButton: !deviceController.initialSetupNoWIFI

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        spacing: 4

        Label {
            text: "Thermostat name"
            font.pointSize: root.font.pointSize
        }

        TextField {
            id: nameTf

            Layout.fillWidth: true
            placeholderText: "Input the name"
            text: appModel?.thermostatName ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
            }
            readOnly: isBusy

            onAccepted: {
                submitBtn.forceActiveFocus();
                submitBtn.clicked();
            }
        }

        Label {
            Layout.fillWidth: true

            width: parent.width
            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.7
            text: "Please ask the customer for the thermostat name. " +
                  "The name is needed to differentiate between thermostats when using the Mobile app."
        }
    }

    BusyIndicator {
        anchors.right: submitBtn.left
        anchors.verticalCenter: submitBtn.verticalCenter

        height: 45
        width: 45
        visible: isBusy
        running: visible
    }

    ContactNuveSupportLabel {
        anchors.bottom: submitBtn.top
        anchors.bottomMargin: 10
        anchors.left: parent.left
        width: parent.width
    }

    //! Submit button
    ButtonInverted {
        id: submitBtn

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        enabled: !isBusy
        text: deviceController.initialSetupNoWIFI ? "Finish" : "Submit"
        visible: !nameTf.activeFocus
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            appModel.thermostatName = nameTf.text;

            if (!deviceController.initialSetupNoWIFI) {
                deviceController.pushInitialSetupInformation();

            } else {
                deviceController.initialSetupFinished();
            }
        }
    }
}

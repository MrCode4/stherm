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
    title: "Thermostat Name"

    /* Children
     * ****************************************************************************************/
    //! Info button in initial setup mode.
    InfoToolButton {
        parent: root.header.contentItem
        visible: initialSetup

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
            text: "Thermostat name"
            font.pointSize: root.font.pointSize * 1.1
        }

        TextField {
            id: nameTf

            Layout.fillWidth: true
            placeholderText: "Input the name"
            text: appModel?.thermostatName ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
            }

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

    //! Submit button
    ButtonInverted {
        id: submitBtn

        //! Has the initial flow been submitted?
        //! TODO: When thermostatName changed, the model should be submitted again.
        property bool submitted: false

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        text: "Submit"
        visible: !nameTf.activeFocus
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            appModel.thermostatName = nameTf.text;

            if (initialSetup) {
                deviceController.updateEditMode(AppSpec.EMGeneral);
                deviceController.saveSettings();
            }

            submitBtn.submitted = true;
        }
    }

    //! Start a timer once they are in technician page and check hasClient (checkSN) every 30 seconds
    Timer {
        repeat: true
        running: root.visible && initialSetup && submitBtn.submitted
        interval: 30000

        onTriggered: {
            deviceController.deviceControllerCPP.checkSN();
        }
    }
}

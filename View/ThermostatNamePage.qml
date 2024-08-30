import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ThermostatNamePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    implicitHeight: implicitHeaderHeight * 6 + _nameTf.implicitHeight + topPadding + bottomPadding
    title: "Thermostat Name"

    /* Children
     * ****************************************************************************************/

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

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemModePage provides a ui for setting system mode
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! I_Device
    property I_Device       device: deviceController?.device

    property bool heatAvailable: device.systemSetup.systemType != AppSpecCPP.CoolingOnly
    property bool coolAvailable: device.systemSetup.systemType != AppSpecCPP.HeatingOnly

    /* Object properties
     * ****************************************************************************************/
    title: "System Mode"

    // Check
    Component.onCompleted: deviceController.updateEditMode(AppSpec.EMSystemMode);

    /* Children
     * ****************************************************************************************/
    //! Make buttons mutually-exclusive
    ButtonGroup {
        property Button previousButton: null

        id: _buttonsGrp

        buttons: _buttonsLay.children

        onCheckedButtonChanged: {
            if (checkedButton !== _vacationButton) {
                previousButton = checkedButton;
            }
        }
    }

    ColumnLayout {
        id: _buttonsLay
        anchors.centerIn: parent
        width: parent.width * 0.5
        spacing: 12

        Button {
            id: _coolingButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemSetup.systemMode === AppSpecCPP.Cooling
            enabled: coolAvailable && !device?.systemSetup._isSystemShutoff
            text: "Cooling"

            onClicked: {
                deviceController.setSystemModeTo(AppSpecCPP.Cooling);
                backButtonCallback();
            }
        }

        Button {
            id: _heatingButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemSetup.systemMode === AppSpecCPP.Heating
            enabled: heatAvailable && !device?.systemSetup._isSystemShutoff
            text: "Heating"

            onClicked: {
                deviceController.setSystemModeTo(AppSpecCPP.Heating);
                backButtonCallback();
            }
        }

        Button {
            id: _autoButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemSetup.systemMode === AppSpecCPP.Auto
            enabled: coolAvailable && heatAvailable && !device?.systemSetup._isSystemShutoff
            text: "Auto"

            onClicked: {
                deviceController.setSystemModeTo(AppSpecCPP.Auto);
                backButtonCallback();
            }
        }

        Button {
            id: _vacationButton

            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemSetup.isVacation && !device?.systemSetup._isSystemShutoff
            enabled: !device?.systemSetup._isSystemShutoff
            text: "Vacation"

            onClicked: {
                //! Push VacationModePage to StackView
                if (_root.StackView.view) {
                    _root.StackView.view.push(_vacationPageCompo);
                }
            }
        }

        Button {
            id: _offButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemSetup.systemMode === AppSpecCPP.Off || device?.systemSetup._isSystemShutoff
            text: "OFF"

            onClicked: {
                deviceController.setSystemModeTo(AppSpecCPP.Off);
                backButtonCallback();
            }
        }
    }

    Component {
        id: _vacationPageCompo

        VacationModePage {
            uiSession: _root.uiSession

            onSaved: {
                _buttonsGrp.previousButton = null

                // Show vacation view page
                uiSession.showMainWindow = false;

                //! Go back twice
                if (_root.StackView.view) {
                    _root.StackView.view.pop();
                    _root.StackView.view.pop();
                }
            }
            onCanceled: {
                //! Restore previous mode
                if (_buttonsGrp.previousButton) {
                    _buttonsGrp.previousButton.checked = true;
                }
            }
        }
    }
}

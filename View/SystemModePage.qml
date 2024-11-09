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
        width: parent.width * 0.65
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
                checkAndUpdateSystemMode(AppSpecCPP.Cooling);
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
                checkAndUpdateSystemMode(AppSpecCPP.Heating);
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
                checkAndUpdateSystemMode(AppSpecCPP.Auto);
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

        //! Emergency Heat
        Button {
            id: emergencyHeatButton

            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true

            //! When users select Manual for emergency heating, an additional button labeled Emergency appears in the System Mode menu
            visible: device?.systemSetup.systemType === AppSpec.HeatPump && device?.systemSetup.emergencyControlType === AppSpec.ECTManually

            checked: device?.systemSetup.systemMode === AppSpecCPP.EmergencyHeat  && !device?.systemSetup._isSystemShutoff
            text: "Emergency Heat"

            onClicked: {
                checkAndUpdateSystemMode(AppSpecCPP.EmergencyHeat);
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
                checkAndUpdateSystemMode(AppSpecCPP.Off);
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

    ConfirmPopup {
        id: confirmPopup

        topPadding: 10
        title: "Change System Mode?"
        message: "Warning!"
        rejectText: "Cancel"
        acceptText: "Ok"
        // this will call reject always even after accept! but this is disconnected in such cases
        // this is for when user close pop up or auto close
        onHid: rejected()
    }


    /* Functions
     * ****************************************************************************************/

    //! Check new system mode has conflict with schedules or not.
    function checkAndUpdateSystemMode(systemMode : int) {
        if (uiSession.schedulesController.findIncompatibleSchedules(systemMode).length > 0) {

            if (device.systemSetup.systemMode === AppSpecCPP.Off) {
                confirmPopup.detailMessage = `There are active Schedule(s)`

            } else {
                confirmPopup.detailMessage = `There are active Schedule(s) configured for ${AppSpec.systemModeToString(device.systemSetup.systemMode)} mode`
            }

            confirmPopup.detailMessage += ` that are incompatible with the new ${AppSpec.systemModeToString(systemMode)} mode. These Schedules will be automatically disabled.`
            confirmPopup.accepted.connect(saveAndDisconnect.bind(this, systemMode));
            confirmPopup.rejected.connect(rejectAndDisconnect);
            confirmPopup.open();

        } else {
            save(systemMode);
        }
    }

    //! Reject and sync ui with model and disconnect the confirmPopup
    function rejectAndDisconnect() {
        confirmPopup.accepted.disconnect(saveAndDisconnect.bind(this));
        confirmPopup.rejected.disconnect(rejectAndDisconnect);

        backToModel();
    }

    //! Back to state of the model
    function backToModel() {
        _coolingButton.checked = device?.systemSetup.systemMode === AppSpecCPP.Cooling;
        _heatingButton.checked = device?.systemSetup.systemMode === AppSpecCPP.Heating;
        _autoButton.checked    = device?.systemSetup.systemMode === AppSpecCPP.Auto;
        _offButton.checked     = device?.systemSetup.systemMode === AppSpecCPP.Off;
        emergencyHeatButton.checked  = device?.systemSetup.systemMode === AppSpecCPP.EmergencyHeat;
    }

    //! Save the systemMode and disconnect the confirmPopup
    function saveAndDisconnect(systemMode : int) {
        confirmPopup.accepted.disconnect(saveAndDisconnect.bind(this));
        confirmPopup.rejected.disconnect(rejectAndDisconnect);
        save(systemMode)
    }

    //! Update the system mode
    function save(systemMode : int) {
        if (deviceController.setSystemModeTo(systemMode))
            backButtonCallback();
        else
            backToModel();
    }
}

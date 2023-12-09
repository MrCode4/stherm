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
        width: parent.width * 0.5
        spacing: 12

        Button {
            id: _coolingButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemMode === AppSpec.SystemMode.Cooling
            text: "Cooling"

            onClicked: {
                deviceController.setSystemModeTo(AppSpec.SystemMode.Cooling);
                backButtonCallback();
            }
        }

        Button {
            id: _heatingButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemMode === AppSpec.SystemMode.Heating
            text: "Heating"

            onClicked: {
                deviceController.setSystemModeTo(AppSpec.SystemMode.Heating);
                backButtonCallback();
            }
        }

        Button {
            id: _autoButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemMode === AppSpec.SystemMode.Auto
            text: "Auto"

            onClicked: {
                deviceController.setSystemModeTo(AppSpec.SystemMode.Auto);
                backButtonCallback();
            }
        }

        Button {
            id: _vacationButton

            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemMode === AppSpec.SystemMode.Vacation
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
            checked: device?.systemMode === AppSpec.SystemMode.Off
            text: "OFF"

            onClicked: {
                deviceController.setSystemModeTo(AppSpec.SystemMode.Off);
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

                deviceController.setSystemModeTo(AppSpec.SystemMode.Vacation);

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

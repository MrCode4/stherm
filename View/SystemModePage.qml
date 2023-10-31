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
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            if (deviceController) {
                //! Save system mode and exit
                switch(_buttonsGrp.checkedButton) {
                    case _coolingButton:
                        deviceController.setSystemModeTo(AppSpec.SystemMode.Cooling);
                        break;
                    case _heatingButton:
                        deviceController.setSystemModeTo(AppSpec.SystemMode.Heating);
                        break;
                    case _autoButton:
                        deviceController.setSystemModeTo(AppSpec.SystemMode.Auto);
                        break;
                    case _vacationButton:
                        deviceController.setSystemModeTo(AppSpec.SystemMode.Vacation);
                        break;
                    case _offButton:
                        deviceController.setSystemModeTo(AppSpec.SystemMode.Off);
                        break;
                }
            }

            //! Go back to previous page
            if (_root.StackView.view) {
                _root.StackView.view.pop();

                if (_buttonsGrp.checkedButton === _vacationButton) {
                    _root.StackView.view.pop(); //! To pop ApplicationMenu
                }
            }
        }
    }

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
        }

        Button {
            id: _heatingButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemMode === AppSpec.SystemMode.Heating
            text: "Heating"
        }

        Button {
            id: _autoButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: device?.systemMode === AppSpec.SystemMode.Auto
            text: "Auto"
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
            text: "Off"
        }
    }

    Component {
        id: _vacationPageCompo

        VacationModePage {
            uiSession: _root.uiSession

            onSaved: _buttonsGrp.previousButton = null
            onCanceled: {
                //! Restore previous mode
                if (_buttonsGrp.previousButton) {
                    _buttonsGrp.previousButton.checked = true;
                }
            }
        }
    }
}

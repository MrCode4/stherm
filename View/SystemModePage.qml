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

    /* Object properties
     * ****************************************************************************************/
    title: "System Mode"

    /* Children
     * ****************************************************************************************/
    //! Make buttons mutually-exclusive
    ButtonGroup {
        id: _buttonsGrp
        buttons: _buttonsLay.children
    }

    ColumnLayout {
        id: _buttonsLay
        anchors.centerIn: parent

        Button {
            id: _coolingButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            checked: true
            text: "Cooling"
        }

        Button {
            id: _heatingButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            text: "Heating"
        }

        Button {
            id: _autoButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            text: "Auto"
        }

        Button {
            id: _vacationButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            text: "Vacation"

            onClicked: {
                //! Push VacationModePage to StackView
                if (_root.StackView.view) {
                    _root.StackView.view.push("qrc:/Stherm/View/SystemMode/VacationModePage.qml", {
                                                  "uiSession": uiSession
                                              });
                }
            }
        }

        Button {
            id: _offButton
            Layout.fillWidth: true
            leftPadding: 24
            rightPadding: 24
            checkable: true
            text: "Off"
        }
    }
}

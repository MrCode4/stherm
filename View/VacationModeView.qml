import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * VacationModeView provides a view for device vation mode
 * ***********************************************************************************************/
Popup {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! UiSession
    property UiSession              uiSession

    //! I_DeviceController
    property I_DeviceController     deviceController: uiSession?.deviceController ?? null

    //! I_Device
    property I_Device               device: uiSession?.appModel ?? null

    //! UiPreferences
    property UiPreferences          uiPreference: uiSession?.uiPreferences ?? null

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size
    modal: true
    dim: true
    closePolicy: "NoAutoClose"
    background: null

    onVisibleChanged: {
        if (visible) {
            welcomeLabel.visible   = false;
            vacationModePop.counter = 5;
        }
    }

    Material.theme: Material.Dark
    /* Children
     * ****************************************************************************************/
    Pane {
        id: vacationModePop

        anchors.fill: parent
        visible: _root.visible

        //! Add count down
        property int counter: 0

        Label {
            id: timerLabel

            text: vacationModePop.counter
            visible: vacationModePop.counter > 0
            anchors.centerIn: parent

            font.pointSize: Qt.application.font.pointSize * 2.5
        }

        ButtonInverted {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 40

            font.bold: true
            visible: vacationModePop.counter > 0
            text: "Cancel"

            onClicked: {
                //! Set system mode to Auto
                if (deviceController) {
                    deviceController.setVacationOn(false);
                }
            }
        }


        Timer {
            running: vacationModePop.counter > 0
            repeat: true

            interval: 1000
            onTriggered: vacationModePop.counter--;

        }


        Label {
            id: welcomeLabel

            font.bold: true
            //! Nuve must be replaced with contractor name
            text: "Nuve\nwelcomes you at home!\nHope you enjoyed your trip."
            visible: vacationModePop.counter > 0
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter

            font.pointSize: Qt.application.font.pointSize
        }

        ButtonInverted {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 40

            font.bold: true
            visible: welcomeLabel.visible
            text: "OK"

            onClicked: {
                //! Set system mode to Auto
                if (deviceController) {
                    deviceController.setVacationOn(false);
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 8
            visible: vacationModePop.counter <= 0 && !welcomeLabel.visible

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                text: "Vacation Mode is On"
            }

            //! Current temprature
            Label {
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: Qt.application.font.pointSize * 2.5
                text: Number(Utils.convertedTemperature(device?.currentTemp ?? 0, device?.setting?.tempratureUnit))
                      .toLocaleString(locale, "f", 0)
            }

            //! Date and time
            DateTimeLabel {
                Layout.alignment: Qt.AlignHCenter
            }

            ButtonInverted {
                font.bold: true
                text: "Resume Normal Operation"
                Layout.alignment: Qt.AlignHCenter

                onClicked: {
                    //! Set system mode to Auto
                    welcomeLabel.visible = true;

                }
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
    }
}

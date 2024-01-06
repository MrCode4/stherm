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
            welcomeLabel.visible    = false;
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
            visible: vacationModePop.counter > 0 && !device.systemSetup.isVacation
            anchors.centerIn: parent

            font.pointSize: Qt.application.font.pointSize * 2.5
        }

        ButtonInverted {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 40

            font.bold: true
            visible: vacationModePop.counter > 0 && !device.systemSetup.isVacation
            text: "Cancel"

            onClicked: {
                if (device.systemSetup.isVacation)
                    deviceController.setVacationOn(false);

                uiSession.showMainWindow = true;
            }
        }


        Timer {
            running: _root.visible && vacationModePop.counter > 0 && !device.systemSetup.isVacation
            repeat: true

            interval: 1000
            onTriggered: {

                vacationModePop.counter--;
                if (deviceController && vacationModePop.counter <= 0) {
                     deviceController.setVacationOn(true);
                }
            }
        }


        Label {
            id: welcomeLabel

            font.bold: true
            //! Nuve must be replaced with contractor name
            text: device.contactContractor.brandName + "\nwelcomes you at home!\nHope you enjoyed your trip."
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
                //! Set system mode to Previous state
                if (deviceController) {
                    deviceController.setVacationOn(false);
                }

                uiSession.showMainWindow = true;
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 8
            visible: device.systemSetup.isVacation && !welcomeLabel.visible

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

                Label {
                    id: unitLbl
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.horizontalCenterOffset: (parent.contentWidth + width) / 2 + 6
                    anchors.top: parent.top
                    anchors.topMargin: -10

                    opacity: 0.6
                    font.pointSize: Qt.application.font.pointSize * 1.2
                    font.capitalization: Font.AllUppercase

                    text: `\u00b0${(device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C")}`
                }
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

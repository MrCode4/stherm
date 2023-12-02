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

    Material.theme: Material.Dark
    /* Children
     * ****************************************************************************************/
    Pane {
        id: _vacationModePop

        anchors.centerIn: parent
        visible: _root.visible

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

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
                Layout.fillWidth: true
                font.bold: true
                text: "Resume Normal Operation"

                onClicked: {
                    //! Set system mode to Auto
                    if (deviceController) {
                        deviceController.setSystemModeTo(AppSpecCPP.Auto);
                    }
                }
            }
        }
    }
}

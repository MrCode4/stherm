import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Stherm

/*! ***********************************************************************************************
 * SwitchHeatingPopup: Use to switch between furnace and heat pump in the dual fuel heating
 * ************************************************************************************************/
I_PopUp {
    id: popup

    /* Property declaration
     * ****************************************************************************************/
    property DeviceController deviceController
    property string           detailMessage: "The thermostat is currently offline, and " +
                                             (deviceController.dfhSystemMode === AppSpec.HeatPump ? "your heat pump is managing the heating.\n\nWould you like to switch to heating through the furnace instead?" :
                                                                                                    "your furnace is managing the heating.\n\nWould you like to switch to heating through the heat pump instead?")

    /* signals
     * ****************************************************************************************/
    signal openSystemModePage();

    /* Object Properties
     * ****************************************************************************************/
    title: ""
    closeButtonEnabled: false
    titleBar: false

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay

        anchors.fill: parent
        anchors.topMargin: 20
        anchors.bottomMargin: 20

        spacing: 16

        Text {
            id: warrantyReplacementText

            Layout.alignment: Qt.AlignRight

            text: qsTr("System modes")
            font.underline: true
            font.pointSize: Qt.application.font.pointSize * 0.9
            color: "#43E0F8"

            TapHandler {
                onTapped: {
                    openSystemModePage();
                    close();
                }
            }
        }

        //! Icon
        Image {
            Layout.alignment: Qt.AlignHCenter
            width: 40
            height: 40
            source: "qrc:/Stherm/Images/swap.svg"
        }

        Label {
            Layout.fillWidth: true
            visible: detailMessage.length > 0
            text: detailMessage
            font.pointSize: Qt.application.font.pointSize * 0.75
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.margins:  16

            ButtonInverted {
                implicitWidth: 100
                text: "Cancel"

                onClicked: {
                    close();
                }
            }

            Item {
                height: 20
                Layout.fillWidth: true
            }

            ButtonInverted {
                implicitWidth: 100
                text: "Switch"

                onClicked: {
                    var switchTo = AppSpec.HeatPump;
                    if (deviceController.dfhSystemMode === AppSpec.HeatPump){
                        switchTo = AppSpec.Conventional;
                    }

                    deviceController.deviceControllerCPP.switchDFHActiveSysType(switchTo);
                    close();
                }
            }
        }
    }
}

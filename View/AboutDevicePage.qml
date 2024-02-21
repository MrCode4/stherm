import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as Template

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AboutDevicePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property I_DeviceController     deviceController: uiSession?.deviceController ?? null

    //! System, use in update notification
    property System                 system:           deviceController.deviceControllerCPP.system

    property int testCounter: 0

    property string appVesion: ""

    /* Object properties
     * ****************************************************************************************/
    title: "Device Info"

    Component.onCompleted: {
        const versionArray = Application.version.split('.')
        const versionArrayMain = versionArray.splice(0, 3)
        appVesion = versionArrayMain.join('.')
    }

    /* Childrent
     * ****************************************************************************************/
    ListView {
        id: _infoLv

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        anchors.bottomMargin: 5

        clip: true
        model: [
            { "key": "Model",               "value": "Nuve - Samo" },
            { "key": "FCC ID",              "value": "2BBXVSAMOV1" },
            { "key": "Contians FCC ID",     "value": "VPYLB1DX" },
            { "key": "IC",                  "value": "LBWA1KL1FX-875" },
            { "key": "Serial No",           "value": system.serialNumber },
            { "key": "Custom Name",         "value": "Living Room" },
            { "key": "URL",                 "value": '<a href="nuvehome.com" style="text-decoration:none;color:#44A0FF;">nuvehome.com</a>' },
            { "key": "E-mail",              "value": '<a href="support@nuvehome.com" style="text-decoration:none;color:#44A0FF;">support@nuvehome.com</link>' },
            { "key": "Software version",    "value": appVesion },
            { "key": "Hardware version",    "value": "01" },
            { "key": "Restart Device",      "value": "01", "type": "button" },
            { "key": "Exit",                "value": "02", "type": "button" },
        ]
        delegate: Item {
            width: ListView.view.width
            height: visible ? Style.delegateHeight * 0.8 : 0
            visible: modelData.key !== "Exit" || system.testMode;

            RowLayout {
                id: textContent
                spacing: 16

                visible: (modelData?.type !== "button") ?? true

                anchors.fill: parent

                Label {
                    Layout.preferredWidth: _fontMetrics.boundingRect("Hardware version :").width + leftPadding + rightPadding
                    font.bold: true
                    text: modelData.key + ":"
                }

                Label {
                    Layout.fillWidth: true
                    font.pointSize: Application.font.pointSize * 0.9
                    textFormat: "RichText"
                    horizontalAlignment: "AlignRight"
                    text: modelData.value

                }
            }

            //! to start test mode Easter Egg
            MouseArea {
                enabled: textContent.visible
                anchors.fill: parent
                onClicked: {
                    if (index === 1) {
                        root.testCounter++;
                        if (root.testCounter === 10) {
                            root.testCounter = 0;
                            if (root.StackView.view) {
                                root.StackView.view.push("qrc:/Stherm/View/Test/TouchTestPage.qml", {
                                                              "uiSession": uiSession
                                                          })
                            }
                        }
                    }
                }
            }

            ButtonInverted {
                id: rebootDevice

                anchors.centerIn: parent
                visible: modelData?.type === "button"
                leftPadding: 8
                rightPadding: 8
                text: modelData.key

                onClicked: {
                    if (modelData.key === "Exit")
                        exitPopup.open();
                    else
                        rebootPopup.open();
                }
            }
        }
    }


    //! Reboot popup with count down timer to send reboot request to system
    RebootDevicePopup {
        id: rebootPopup
        system: root.system
        anchors.centerIn: Template.Overlay.overlay
    }


    //! Exit popup with count down timer to send exit request to system
    ResetDevicePopup {
        id: exitPopup
        system: root.system
        anchors.centerIn: Template.Overlay.overlay
    }

    FontMetrics {
        id: _fontMetrics
    }
}

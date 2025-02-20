import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * IncorrectZipCodePopup prompts user to inform user for incorrect zip code.
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property var uiSession

    property DeviceControllerCPP deviceControllerCPP: uiSession.deviceController.deviceControllerCPP

    property ServiceTitan serviceTitan: uiSession.appModel.serviceTitan

    property bool isValidToKeepOpen: deviceControllerCPP.isNeedOutdoorTemperature &&
                                     deviceControllerCPP.isEligibleOutdoorTemperature &&
                                     !deviceControllerCPP.isZipCodeValid

    //! Close the popup when the popup is not valid to keep it open due to condition changes
    onIsValidToKeepOpenChanged: {
        if (!isValidToKeepOpen)
            close();
    }

    /* Object properties
     * ****************************************************************************************/
    title: ""
    keepOpen: true

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay

        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.fillWidth: true

            font.pointSize: Qt.application.font.pointSize * 0.8
            text: {
                var userInfo = "The zip code provided during installation may be incorrect.\n\n";

                if (serviceTitan && serviceTitan.zipCode.length > 0) {
                    userInfo += `To ensure accurate outdoor temperature readings, please verify the following zip code: ${serviceTitan.zipCode}`;

                } else {
                    userInfo += "The zip code is currently missing. To get accurate outdoor temperature readings, please update the zip code."
                }

                return userInfo;
            }

            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 24
            spacing: 24

            ButtonInverted {
                Layout.fillWidth: true
                text: "Cancel"

                onClicked: {
                    close();
                }
            }

            ButtonInverted {
                Layout.fillWidth: true
                text: "Update"

                onClicked: {
                    //! Go to the update zip code page.
                    uiSession.openPageFromHome("qrc:/Stherm/View/Menu/ZipCodeEditPage.qml");
                    close();
                }
            }
        }
    }
}

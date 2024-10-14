import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InitialFlowErrorPopup: Show initial flow errors.
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Error text
    property string errorMessage: ""

    property DeviceController deviceController

    /* Object properties
     * ****************************************************************************************/
    title: "Error"
    width: AppStyle.size * 0.85
    height: AppStyle.size * 0.85

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        width: parent?.width * 0.95 ?? 0
        anchors.centerIn: parent
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: Style.button.buttonHeight

            Label {
                text: "Serial Number: "
                horizontalAlignment: Text.AlignLeft
                font.pointSize: Application.font.pointSize * 0.85
            }

            Label {
                text: deviceController.system.serialNumber
                horizontalAlignment: Text.AlignLeft
                font.pointSize: Application.font.pointSize * 0.7
            }
        }

        Flickable {
            id: errorFlick

            Layout.preferredHeight: root.height * 0.4
            Layout.fillWidth: true

            ScrollIndicator.vertical: ScrollIndicator {
                parent: errorFlick
                height: parent.height
                x: parent.width
            }

            clip: true
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: width
            contentHeight: errorLayout.implicitHeight

            Label {
                id: errorLayout

                anchors.fill: parent

                color: AppStyle.primaryRed
                text: errorMessage
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Application.font.pointSize * 0.7
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.preferredHeight: 35

            font.pointSize: Application.font.pointSize * 0.65
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:Text.AlignBottom
            text: "Contact Nuve Support: (657) 626-4887 for issues."
        }


        //! Connect/Disconnect button
        ButtonInverted {
            text: "Send log"

            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                if (NetworkInterface.hasInternet)
                    logBusyPop.open();
                else
                    deviceController.system.alert("No Internet, Please check your connection before sending log.")
            }
        }
    }

    Popup {
        id: logBusyPop

        anchors.centerIn: parent
        width: Math.max(implicitWidth, root.width * 0.5)
        height: root.height * 0.5
        parent: root.parent
        modal: true

        onOpened: {
            //! Call sendLog()
            deviceController.system.sendLog();
            close();
        }

        contentItem: Label {
            text: "Preparing log, \nplease wait ..."
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
            lineHeight: 1.4
        }
    }
}


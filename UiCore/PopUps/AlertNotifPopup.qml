import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AlertNotifPopup provides a popup for showing and alert or notification
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Popup Message
    property Message    message

    //! UiSession: To open an item from home
    property UiSession          uiSession

    /* Object properties
     * ****************************************************************************************/
    title: message ? ((message.type === Message.Type.Alert ||
                       message.type === Message.Type.SystemAlert ||
                       message.type === Message.Type.SystemNotification) ? "Alert" : "Message"
                      ) : ""
    icon: message ? (message.icon === ""
                     ? ((message.type === Message.Type.Alert ||
                         message.type === Message.Type.SystemAlert ||
                         message.type === Message.Type.SystemNotification)
                        ? FAIcons.triangleExclamation
                        : (message.type === Message.Type.Error ? FAIcons.circleXmark
                                                               : FAIcons.bell)
                        )
                     : message.icon)
                  : ""

    spacing: 10
    bottomPadding: 6
    horizontalPadding: 10

    /* Children
     * ****************************************************************************************/

    width: 350

    GridLayout {
        anchors.fill: parent
        rows: 2
        columns: 2

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            textFormat: Text.MarkdownText
            text: message?.message ?? ""
            wrapMode: "Wrap"
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
        }


        ButtonInverted {
            id: okButton

            Layout.alignment: message.type === Message.Type.Alert ? Qt.AlignRight : Qt.AlignLeft
            Layout.columnSpan: message.type === Message.Type.Alert ? 2 : 1

            Layout.preferredWidth: conatactContractorButton.width
            Layout.preferredHeight: conatactContractorButton.height

            leftPadding: 8
            rightPadding: 8
            text: "    OK    "
            font.pointSize: Qt.application.font.pointSize * 0.8
            visible: (message.type === Message.Type.Alert || message.type === Message.Type.SystemAlert)

            onClicked: {
                close()
            }
        }

        ButtonInverted {
            id: conatactContractorButton
            Layout.alignment: Qt.AlignRight

            leftPadding: 8
            rightPadding: 8
            text: "Conatact\nContractor"
            font.pointSize: Qt.application.font.pointSize * 0.8

            visible: message.type === Message.Type.SystemAlert

            onClicked: {
                close();

                //! Open ContactContractorPage
                uiSession.popUps.openPageFromHome("qrc:/Stherm/View/ContactContractorPage.qml");

            }
        }
    }
}

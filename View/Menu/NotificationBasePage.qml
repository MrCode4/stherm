import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

BasePageView {
    id: root

    property var filters: []
    property MessageController  messageController: uiSession.messageController

    ListView {
        spacing: 4
        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        model: {
            let allItems = appModel?.messages ?? [];
            let filteredItems = [];
            if (allItems?.length == 0 || filters?.length == 0) {
                filteredItems = allItems;
            }
            else {
                allItems.forEach(message => {if (filters.indexOf(message.type) >= 0) filteredItems.push(message);});
            }

            return filteredItems;
        }

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }        

        delegate: ItemDelegate {
            id: itemDelegate
            width: ListView.view.width
            height: Material.delegateHeight
            property Message message: (modelData instanceof Message ? modelData : null)
            property string itemTitle: {
                switch (message.type) {
                case  Message.Type.Alert:
                case Message.Type.SystemAlert:
                    return "Alert"
                default:
                    return  "Message"
                }
            }

            property string itemIcon: {
                switch (message.type) {
                    case  Message.Type.Alert:
                    case Message.Type.SystemAlert:
                        return "\uf071" // triangle-exclamation icon
                    case Message.Type.Notification:
                        return "\uf0f3" //! bell icon
                    default:
                        return  ""
                }
            }

            highlighted: !message.isRead
            text: itemDelegate.itemTitle
            contentItem: RowLayout {
                spacing: 6
                RoniaTextIcon {
                    Layout.alignment: Qt.AlignCenter
                    text:itemDelegate.itemIcon
                }

                Label {
                    id: messageTypeLabel
                    Layout.leftMargin: 10
                    Layout.alignment: Qt.AlignCenter
                    text: itemDelegate.itemTitle
                    elide: "ElideRight"
                }

                Label {
                    Layout.alignment: Qt.AlignCenter
                    Layout.fillWidth: true
                    text: message.datetime ? `(${DateTimeManager.utcDateTimeToLocalString(message.datetime)})` : " -"
                    elide: Qt.ElideRight
                    font.pixelSize: messageTypeLabel.font.pixelSize - 2
                }
            }

            onClicked: {
                if (messageController)
                    messageController.showMessage(message);
            }
        }
    }
}

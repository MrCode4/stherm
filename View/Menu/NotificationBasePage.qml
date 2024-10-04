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

                    property string dateTimeFormat: "yyyy-MM-dd HH:mm:ss"

                    property string dateTimeString: {
                        if (message.datetime.length > 0) {
                            var dts = DateTimeManager.utcDateTimeToLocalString(message.datetime, dateTimeFormat);

                            // If QDateTime could not convert the date time with the dateTimeFormat, it use the ISO (`yyyy-MM-ddTHH:mm:ss.zzz`) instead.
                            // It handle the old server messages.
                            if (dts.length === 0) {
                                dts = DateTimeManager.utcDateTimeToLocalString(message.datetime, "yyyy-MM-ddTHH:mm:ss.zzz");
                            }

                            return dts;
                        }

                        return "";
                    }

                    text: dateTimeString.length > 0 ? ` (${dateTimeString})` : " -"
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

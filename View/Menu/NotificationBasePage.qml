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
            id: scrollIndicator

            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }        

        //! Use for gradient in the menu
        Rectangle {
            width: root.width
            height:  root.height * 0.45
            y:  ((scrollIndicator.position + scrollIndicator.size) < 0.97) ? root.height * 0.55 :  root.height


            gradient: Gradient {
                GradientStop { position: 0; color: Qt.alpha(AppStyle.backgroundColor, 0.0) }
                GradientStop { position: 1; color: Qt.alpha(AppStyle.backgroundColor, 1.0) }
            }

            //! Attach the animations
            //! Behaviour on y
            Behavior on y {
                enabled : ((scrollIndicator.position + scrollIndicator.size) > 0.97)
                NumberAnimation {
                    duration: 1000
                    easing.type: Easing.OutCubic
                }
            }
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

                    property string inputDateTimeFormat: "yyyy-MM-dd HH:mm:ss"
                    property string outputDateTimeFormat: (appModel.setting.timeFormat === AppSpec.TimeFormat.Hour24) ? "MMM dd, yyyy hh:mm" : "MMM dd, yyyy h:mmAP"

                    property string dateTimeString: {
                        if (message.datetime.length > 0) {
                            var dts = DateTimeManager.utcDateTimeToLocalString(message.datetime, inputDateTimeFormat, outputDateTimeFormat);

                            // If QDateTime could not convert the date time with the dateTimeFormat, it use the ISO (`yyyy-MM-ddTHH:mm:ss.zzz`) instead.
                            // It handle the old server messages.
                            if (dts.length === 0) {
                                dts = DateTimeManager.utcDateTimeToLocalString(message.datetime, "yyyy-MM-ddTHH:mm:ss.zzz", outputDateTimeFormat);
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

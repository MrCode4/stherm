import QtQuick
import Stherm

NotificationBasePage {
    id: root
    title: "Messages"
    filters: [Message.Type.Unknown, Message.Type.Notification, Message.Type.Error]
}

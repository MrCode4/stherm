import QtQuick
import Stherm

NotificationBasePage {
    id: root
    title: "System Alerts"
    filters: [Message.Type.SystemNotification, Message.Type.SystemAlert]
}

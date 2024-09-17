import QtQuick
import Stherm

NotificationBasePage {
    id: root
    title: "Alerts"
    filters: [Message.Type.Unknown, Message.Type.Error, Message.Type.Alert]
}

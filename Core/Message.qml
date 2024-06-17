import QtQuick

import QtQuickStream
import Stherm

QSObject {
    id: _root

    /* Enums
     * ****************************************************************************************/
    enum Type {
        Unknown,
        Alert,
        Notification,
        SystemNotification, // like No Wi-Fi connection and No internet connection
        SystemAlert,        // All alerts that related to contractor, Sensor malfunction alerts (Crucial alerts)
        Error
    }

    enum SourceType {
        Unknown = 0,
        Device  = 1,
        Server  = 2
    }

    /* Property declaration
     * ****************************************************************************************/
    //! Type of Message
    property int        type:       Message.Type.Unknown

    property int        sourceType: Message.SourceType.Device

    //! Message text
    property string     message:    ""

    //! Is message read (seen)
    property bool       isRead:     false

    //! Icon for this message, it should be a unicode from FontAwsome (can be blank)
    property string     icon:       ""

    //! Date of arrival
    property string     datetime

}

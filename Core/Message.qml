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

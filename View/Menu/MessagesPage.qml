import QtQuick
import Stherm

NotificationBasePage {
    id: root

    title: "Messages"
    filters: [Message.Type.Notification]

    enableTitleTap: true

    onTitleLongTapped: {
        deleteConfirmPopup.open();
    }


    //! Remove messages
    ConfirmPopup {
       id: deleteConfirmPopup

        message: "Delete all messages"
        detailMessage: "Are you sure you want to delete all messages?"
        onAccepted: {
            var hasMessagesForDelete = root.filteredItems.length > 0;
            root.filteredItems.forEach(msg => {
                                           msg._qsRepo = null;
                                           var msgId = appModel.messages.findIndex(elem => elem.id === msg.id);
                                           if (msgId > -1) {
                                               appModel.messages.splice(msgId, 1);
                                           }
                                       });

            if (hasMessagesForDelete) {
                appModel.messagesChanged();
            }
        }
    }
}

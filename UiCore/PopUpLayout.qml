import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * The PopUpLayout handles displaying popups as requested
 * ************************************************************************************************/
Item {
    id: popUpLayout

    /* Property declaration
     * ****************************************************************************************/
    //! Is there any popup
    readonly property bool isTherePopup: _internal.popupQueue.length > 0

    property bool mandatoryUpdate: false

    /* Children
     * ****************************************************************************************/
    QtObject {
        id: _internal

        property var    popupQueue: []

        function onPopupClosedDestroyed(popup: I_PopUp)
        {
            var pIndx = popupQueue.findIndex(element => element === popup);
            if (pIndx > -1) {
                popup.hid.disconnect(onPopupClosedDestroyed);
                popup.destructed.disconnect(onPopupClosedDestroyed);

                popupQueue.splice(pIndx, 1);
                popupQueueChanged();
            }

            //! Show next popup in the queue if any
            if (popupQueue.length > 0) {
                popupQueue[0].open();
            }
        }
    }

    /* Functions
     * ****************************************************************************************/
    //! Shows the first popup in the uiSession.popUpQueue
    //! \param hightPriority: if it's true popup will be shown immediately above all visible ones,
    //! and if false, it is added in a queue to be shown after current popup is closed (if any)
    function displayPopUp(popup: I_PopUp, hightPriority=true)
    {
        if (!(popup instanceof I_PopUp)) {
            return;
        }

        popup.hid.connect(_internal.onPopupClosedDestroyed);
        popup.destructed.connect(_internal.onPopupClosedDestroyed);

        if (hightPriority) {
            //! Disply it right away
            _internal.popupQueue.push(popup);
            _internal.popupQueueChanged();
            popup.open();
        } else {
            //! Push it back to the queue
            _internal.popupQueue.push(popup);
            _internal.popupQueueChanged();
            if (_internal.popupQueue.length === 1) {
                popup.open();
            }
        }
    }

    function closeAllPopups()
    {
        _internal.popupQueue.forEach((popup) => {
                                         if (popup instanceof Popup && (!mandatoryUpdate || !(popup instanceof UpdateNotificationPopup))) {
                                             popup.close();
                                         }
                                     })
    }
}

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
            } else {
                console.log("unknown error: ", "isTherePopup : ", isTherePopup, "Length: ", _internal.popupQueue.length, "popupobjectName: ", popup.objectName)
                // reevaluate and break the bind?
            }

            //! Show next popup in the queue if any
            if (popupQueue.length > 0) {
                popupQueue[0].open();
            }

            // checkScreenSaver();
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

        // If the popup is already in the popup queue, we close it and reopen it with a new priority.
        // This prevents signal disconnections in the `onPopupClosedDestroyed` function when the first popup is closed.
        var pIndx = _internal.popupQueue.findIndex(element => element === popup);
        if (pIndx > -1) {
            popup.close();
        }

        popup.hid.connect(_internal.onPopupClosedDestroyed);
        popup.destructed.connect(_internal.onPopupClosedDestroyed);

        if (hightPriority) {
            //! Disply it right away
            _internal.popupQueue.unshift(popup);
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

        // checkScreenSaver();
    }

    function closeAllPopups()
    {
        _internal.popupQueue.forEach((popup) => {
                                         //! Avoid to close the UpdateNotificationPopup on mandatory update mode
                                         //! Avoid to close the AlertNotifPopup
                                         if (popup instanceof Popup && (popup?.keepOpen ?? false) === false) {
                                             popup.close();
                                         }
                                     })
    }

    //! Alerts must be seen from a distance by the user
    //! Unused
    //! The screensaver remains active even when an AlertNotifPopup instance exists.
    //! We display some icons on the screensaver page when an AlertNotifPopup instance exists,
    //! so we need to disable this functionality for now.
    function checkScreenSaver() {
        let alertNotifPopup = _internal.popupQueue.find(popup => (popup instanceof AlertNotifPopup));

        //! This function should not cause problems for TOF and system events.
        if (alertNotifPopup) {
            ScreenSaverManager.setInactive();
        } else {
            ScreenSaverManager.setActive();

        }
    }
}

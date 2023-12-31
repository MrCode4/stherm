import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ExitConfirmPopup prompts user to confirm exiting when changes are not saved.
 * ***********************************************************************************************/
ConfirmPopup {
    id: root

    /* Object properties
     * ****************************************************************************************/
    title: "Exit"
    message: "Do you want to save changes before exiting?"
    detailMessage: "Changes are lost if not saved."
    acceptText: "Save"
    rejectText: "No"
}

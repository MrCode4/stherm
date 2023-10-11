import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QTemplateProject

/*! ***********************************************************************************************
 * UiSessionPopups is a helper class to contain all the popups that can be called throughout the
 * RIG GUI (but relate to the session). This way we only need to instantiate one instance of the
 * same popup, which should improve performance, and, most of all, warrant consistency.
 *
 * \todo The current implementation is limited/questionable as uiSession cannot be passed to the
 *       popups. In reality, the popups belog the the QTemplateProject, not to the session, and should be
 *       moved there. The question then remains how the communication should work.
 *
 * \todo A better API could allow any class to call uiSession.popUps.xyz.show(), or to simply call
 *       uiSession.actions.doSomething(args);
 *
 * \todo The Popups should be passed the uiSession and handle their own actions instead of leaving
 *       that to the parent class.
 * ************************************************************************************************/
Item {
    /* Popup Functions
     * ****************************************************************************************/
    function popupAbout()
    {
        showPopUp(aboutPopup);
    }

    /* Children
     * ****************************************************************************************/
    AboutPopUp {
        id: aboutPopup
    }

}

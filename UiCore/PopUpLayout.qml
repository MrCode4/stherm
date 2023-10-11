import QtQuick 2.0

import QTemplateProject
/*! ***********************************************************************************************
 * The PopUpLayout listens to the UiSession and shows any popups that might be pending. We use a
 * queued (first-in-first-out) approach.
 * ************************************************************************************************/
Rectangle {
    id: popUpLayout

    /* Property Declarations
     * ****************************************************************************************/
    property UiSession  uiSession
    property I_PopUp    currentPopUp: null

    /* Object Properties
     * ****************************************************************************************/
    color: "#aa000000"
    visible: false

    /* Functions
     * ****************************************************************************************/
    //! Hides the popup and shows shows the first queued one from uiSession.popUpQueue
    function closePopUp(popup) {
        // Hide the current popup
        currentPopUp.visible            = false;
        currentPopUp                    = null;

        // Show any pending popups
        if (uiSession.popUpQueue.length !== 0) {
            displayPopUp(uiSession.popUpQueue[0]);
        }
        // Otherwise hide the layout
        else {
            popUpLayout.visible = false;
        }
    }

    //! Shows the first popup in the uiSession.popUpQueue
    function displayPopUp(popup) {
        // Sanity Check: show the first popup in the queue
        if (uiSession.popUpQueue.indexOf(popup) !== 0) {
            return;
        }

        // Show the new popup
        currentPopUp                    = popup;
        currentPopUp.parent             = popUpLayout;
        currentPopUp.anchors.centerIn   = popUpLayout;
        currentPopUp.visible            = true;

        closeButton.anchors.horizontalCenter = currentPopUp.right;
        closeButton.anchors.verticalCenter   = currentPopUp.top;

        popUpLayout.visible  = true;
    }

    /* Connections
     * ****************************************************************************************/
    Connections {
        target: uiSession
        function onSigShowPopUp(popup) {
            popUpLayout.displayPopUp(popup);
        }
    }

    Connections {
        target: uiSession
        function onSigHidePopUp(popup) {
            popUpLayout.closePopUp(popup);
        }
    }

    /* Children
     * ****************************************************************************************/
    // Sink for all mouse events outside the popup
    MouseArea {
        anchors.fill: parent
        enabled: currentPopUp !== null && currentPopUp.visible
    }

    RoniaIconButtonRound {
        id: closeButton

        size: 48
        z: 100

        backColor: "darkred"
        text: "\uf00d"

        onClicked: {
            uiSession.hidePopUp(currentPopUp);
        }
    }
}

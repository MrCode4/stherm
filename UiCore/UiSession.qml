import QtQuick 2.0
import Qt.labs.platform

import QTemplateProject

/*! ***********************************************************************************************
 * The UiSession contains all information required by graphical components to display the right
 * state. Additionally, it keeps track of the user's movement through the UI and serves as the
 * mechanism to enable (generic/high-level) communication between components, e.g., to display
 * popups.
 * ************************************************************************************************/
QtObject {
    /* Property Declarations
     * ****************************************************************************************/
    enum UserLevel {
        USER            = 1,
        OPERATOR        = 2,
        TECHNICIAN      = 3,
        DEALER          = 4,
        SUPER_DEALER    = 5,
        VENDOR          = 6,
        VENDOR_TECH     = 8,
        DEVELOPER       = 10
    }

    /* Property Declarations
     * ****************************************************************************************/
    // Debug or not
    property bool               debug:          userLevel >= UiSession.UserLevel.DEVELOPER

    // Logged in user level
    property int                userLevel:      UiSession.UserLevel.USER

    // Stack of current panels
    //! \todo this should probably be a custom type with extra props
    readonly property var       panelStack:     []

    // Stack of current popups
    //! \todo this should probably be a custom type with extra props
    readonly property var       popUpQueue:     []

    // Popups that can be called for this UiSession
    readonly property UiSessionPopups popUps:   UiSessionPopups {}

    //! Ui Preferences (Units, etc)
    property UiPreferences      uiPreferences:  UiPreferences {}

    //! app core
    property I_Model            appModel:        AppCore.model

    //! Path of File to save model.
    property string             currentFile:    ""

    /* Connections
     * ****************************************************************************************/

    /* Signals
     * ****************************************************************************************/
    signal sigShowPanel (I_Panel panel);
    signal sigHidePanel (I_Panel panel);
    signal sigShowPopUp (I_PopUp popUp);
    signal sigHidePopUp (I_PopUp popUp);

    /* Signal Handlers
     * ****************************************************************************************/

    /* Functions
     * ****************************************************************************************/
    //! Closes all panels, e.g., when home button is pressed, or mode is changed
    function hideAllPanels() {
        while (uiSession.panelStack.length > 0) {
            hidePanel(uiSession.panelStack[0]);
        }
    }

    //! Indicates that a panel needs to be closed (the PanelLayout handles this)
    function hidePanel(panel) {
        panelStack.splice(panelStack.lastIndexOf(panel));
        sigHidePanel(panel);
    }

    //! Indicates that a popup needs to be closed (the PopUpLayout handles this)
    function hidePopUp(popUp) {
        popUpQueue.splice(popUpQueue.indexOf(popUp), 1);
        sigHidePopUp(popUp);
    }

    //! Goes to default UiMode or UnavilableUiMode, depending on line availability
    function resetSession() {
        currentMode = !smartLine.sosIsAvailable
                    ? UiModeRegister.unavailableMode
                    : defaultMode;
    }

    //! Indicates that a panel needs to be shown (the PanelLayout handles this)
    function showPanel(panel) {
        // Sanity check: skip if we already have the panel on the stack
        if (panelStack.indexOf(panel) === -1) {
            panelStack.push(panel);
        }

        sigShowPanel(panel);
    }

    //! Indicates that a popup needs to be shown (the PopUpLayout handles this)
    function showPopUp(popUp) {
        // Sanity check: don't allow duplicates
        if (popUpQueue.indexOf(popUp) !== -1) {
            return;
        }

        popUpQueue.push(popUp);
        sigShowPopUp(popUp);
    }

    function loadUiPreferences() : bool
    {
        console.log("[UiSession] Failed to load the UiPreferences")
        return false;

    }

    function writeUiPreferences() : bool
    {
        console.log("[UiSession] Saving failed, aborting")
        return false;

    }

    Component.onCompleted: {
        if (!loadUiPreferences()) {
            console.log("failed to load the UiPreferences")
            writeUiPreferences();
        }
    }
}

import QtQuick 2.0
import Qt.labs.platform

import Stherm
import Ronia
/*! ***********************************************************************************************
 * The UiSession contains all information required by graphical components to display the right
 * state. Additionally, it keeps track of the user's movement through the UI and serves as the
 * mechanism to enable (generic/high-level) communication between components, e.g., to display
 * popups.
 * ************************************************************************************************/
Item {
    id: root

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
    //! Path of File to save model.
    property string           currentFile:    ""

    //! Set to true when new update available,
    //! the hasUpdateNotification just use in red notification hint in
    //! menu and update page in menu
    property bool             hasUpdateNotification: false

    property bool             hasUnreadAlerts: false
    property bool             hasUnreadMessages: false

    //! Config file path
    readonly property string  configFilePath:   "/usr/local/bin/sthermConfig.QQS.json"

    readonly property string  recoveryConfigFilePath:   "/mnt/recovery/recovery/sthermConfig.QQS.json"

    // Debug or not
    property bool               debug:          userLevel >= UiSession.UserLevel.DEVELOPER


    // Logged in user level
    property int                userLevel:      UiSession.UserLevel.USER

    //! Settings fetch from server at least once
    property bool               settingsReady: false

    //! To switch between main window and vacation page
    property bool               showMainWindow: true

    // Stack of current panels
    //! \todo this should probably be a custom type with extra props
    readonly property var       panelStack:     []

    // Stack of current popups
    //! \todo this should probably be a custom type with extra props
    readonly property var       popUpQueue:     []

    // Popups that can be called for this UiSession
    readonly property UiSessionPopups popUps:   UiSessionPopups {
        uiSession: root
        parent: root
    }

    readonly property bool isAnyPopupVisible: popUps.isAnyPopupOpened || popupLayout.isTherePopup

    //! Ui Preferences (Units, etc)
    property UiPreferences      uiPreferences:  UiPreferences {}

    //! managing popups
    property PopUpLayout        popupLayout

    //Manages displaying toast messages requested from different parts of the app
    property ToastManager toastManager:ToastManager

    //! app core, will be created in main.qml onCompleted function
    property I_Device           appModel

    //! This property can be used to disable wifi refreshing temporarily
    property bool               refreshWifiEnabled: true

    //! Use to block the system mode page when system mode is emergency
    property int remainigTimeToUnblockSystemMode: 0

    //! Retrieve device information at one-second intervals.
    property Timer timer:   Timer {
        running: true
        repeat: true
        interval: AppSpec.readInterval

        onTriggered: {
            deviceController.updateInformation();
        }
    }

    //! Enable test mode from UI (eg: press 10 times on the FCC ID in About page, ...)
    //! Not used for now! maybe usefull for later
    property bool uiTestMode: false

    Component.onCompleted: deviceController.startDeviceRequested();

    /* Controllers
     * ****************************************************************************************/

    //! Device controller
    property I_DeviceController deviceController:   DeviceController {
        device: appModel
        uiSession: root
        schedulesController: root.schedulesController
        messageController: root.messageController
    }

    //! SensorController instance
    property SensorController   sensorController:      SensorController {
        device: appModel
    }

    //! MessageController instance
    property MessageController  messageController:      MessageController {
        uiSession: root
    }

    //! schedulesController instance
    property SchedulesController schedulesController:  SchedulesController {
        deviceController: root.deviceController
    }

    /* Connections
     * ****************************************************************************************/

    /* Signals
     * ****************************************************************************************/
    signal sigShowPanel (I_Panel panel);
    signal sigHidePanel (I_Panel panel);
    signal sigShowPopUp (I_PopUp popUp);
    signal sigHidePopUp (I_PopUp popUp);
    signal requestShowToast(string message);
    signal showHome(); //! This signal can be emitted to request going back to Home
    signal openSystemModePage();
    signal openWifiPage(backButtonVisible : bool, openFromNoWiFiInstallation: bool);
    signal openUnlockPage();
    signal goToInitialSetupNoWIFIMode();

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
}

import QtQuick 2.11
import QtQuick.Dialogs
import Stherm

Item {
    /* Property Declarations
     * ****************************************************************************************/

    property UiSession uiSession

    property I_Device   appModel:     uiSession.appModel

    //! Save a project
    Shortcut {
        sequences: [StandardKey.Save]
        onActivated: {
            uiSession.deviceController.saveSettings()
        }
    }

    //! Save as a project
    Shortcut {
        sequences: [StandardKey.SaveAs, "Ctrl+Shift+S"]
        onActivated: {
            uiSession.deviceController.saveSettings()
        }
    }

    //! Load a project
    Shortcut {
        sequences: [StandardKey.Open]
        onActivated: {
         console.log("Open key is deactivated..");
        }
    }

    /* Functions
     * ****************************************************************************************/
}

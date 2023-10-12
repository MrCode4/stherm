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
            saveDialog.saveProject();
        }
    }

    //! Save as a project
    Shortcut {
        sequences: [StandardKey.SaveAs, "Ctrl+Shift+S"]
        onActivated: {
            uiSession.currentFile = "";
            saveDialog.saveProject();
        }
    }

    //! Load a project
    Shortcut {
        sequences: [StandardKey.Open]
        onActivated: loadDialog.visible = true;
    }

    //Save Dialog
    SaveDialoge {
        id: saveDialog
    }

    //! Load Dialog
    LoadDialoge {
        id: loadDialog
    }

    /* Functions
     * ****************************************************************************************/
}

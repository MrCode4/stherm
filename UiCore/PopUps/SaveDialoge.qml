import QtQuick
import QtQuick.Dialogs
import QTemplateProject

/*! ***********************************************************************************************
 * Save Dialoge
 * ************************************************************************************************/
FileDialog {
    id: saveDialog

    /* Object Properties
     * ****************************************************************************************/
    fileMode: FileDialog.SaveFile
    nameFilters: [ "QtQuickStream Files (*.json)" ]

    onAccepted: {
        // Correct file extention and pass to currect file
        uiSession.currentFile = saveDialog.currentFile;
        uiSession.currentFile = uiSession.currentFile.replace(/^(file:\/{3})/, "")

        // Save file
        AppCore.defaultRepo.saveToFile(uiSession.currentFile);
    }

    //! Save current project.
    function saveProject() {
        if(uiSession.currentFile.length === 0)
            saveDialog.visible  = true
        else
            AppCore.defaultRepo.saveToFile(uiSession.currentFile);
    }
}

import QtQuick
import QtQuick.Dialogs
import QTemplateProject

/*! ***********************************************************************************************
 * Load Dialoge
 * ************************************************************************************************/
FileDialog {
    id: loadDialog

    /* Object Properties
     * ****************************************************************************************/
    fileMode: FileDialog.OpenFile
    nameFilters: [ "QtQuickStream Files (*.json)" ]

    onAccepted: {
        AppCore.defaultRepo._localImports =  [ "QTemplateProject"]

        // Correct file extention and pass to currect file
        uiSession.currentFile = loadDialog.currentFile;
        uiSession.currentFile = uiSession.currentFile.replace(/^(file:\/{3})/, "")

        //  Load file
        AppCore.defaultRepo.loadFromFile(uiSession.currentFile);

        //One SiSCore exist.
        for (var prop in AppCore.defaultRepo._qsObjects) {
            if(AppCore.defaultRepo._qsObjects[prop].qsType === "RoniaModel") {
                uiSession.appModel = AppCore.defaultRepo._qsObjects[prop];;
            }
        }
    }
}

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QTemplateProject


/*! ***********************************************************************************************
 * This is the highest level graphical object, i.e., the main application window. The state
 * of each instance is stored in the UiSession, which needs to be passed to its children.
 * Multiple instances can be created.
 *
 * ************************************************************************************************/
ApplicationWindow {
    id: window

    /* Property Declarations
     * ****************************************************************************************/
    property UiSession uiSession: UiSession { }

    property string    currentFile: uiSession.currentFile

    /* Object Properties
     * ****************************************************************************************/
    width: 1024
    height: 768
    visible: true
//    visibility: uiSession.uiPreferences.windowMode
    title: qsTr("Template" + "               " + currentFile)
    background: Rectangle {
        color: AppStyle.primaryDarkGrey
    }

    //! Create defualt repo and root object to save and load
    Component.onCompleted: {
        AppCore.defaultRepo.initRootObject("QSObject");
    }

    /* Fonts
     * ****************************************************************************************/
    FontLoader { source: "qrc:/QTemplateProject/Fonts/Font Awesome 6 Pro-Thin-100.otf" }
    FontLoader { source: "qrc:/QTemplateProject/Fonts/Font Awesome 6 Pro-Solid-900.otf" }
    FontLoader { source: "qrc:/QTemplateProject/Fonts/Font Awesome 6 Pro-Regular-400.otf" }
    FontLoader { source: "qrc:/QTemplateProject/Fonts/Font Awesome 6 Pro-Light-300.otf" }
    FontLoader { source: "qrc:/QTemplateProject/Fonts/RobotoMono-Regular.ttf" }


    /* Style
     * ****************************************************************************************/
    Material.theme: Material.Dark
    Material.accent: AppStyle.primaryColor

    /* Splash Window
     * ****************************************************************************************/


    /* Toolbars
     * ****************************************************************************************/

    //! Header
    header: Item {}

    footer: Item {}

    /* Children
     * ****************************************************************************************/

    MainView {
        id: mainView
        anchors.fill: parent
        uiSession: window.uiSession
    }

    //! Popup layout
    PopUpLayout {
        id: popUpLayout
        uiSession: window.uiSession
        anchors.fill: parent
        z: 100
    }

    ShortcutManager {
       uiSession: window.uiSession
    }
}

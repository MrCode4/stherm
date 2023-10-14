import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import Stherm

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
    width: 480
    height: 480
    visible: true
//    visibility: uiSession.uiPreferences.windowMode
    title: qsTr("Template" + "               " + currentFile)

    //! Create defualt repo and root object to save and load
    Component.onCompleted: {
        AppCore.defaultRepo.initRootObject("QSObject");
    }

    /* Fonts
     * ****************************************************************************************/
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Thin-100.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Solid-900.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Regular-400.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Light-300.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/RobotoMono-Regular.ttf" }


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

    ScreenSaver {
        id: _screenSaver
        anchors.centerIn: parent
    }

    //! This mouse area is to detect app interactions to prevent screen saver being shown
    MouseArea {
        anchors.fill: parent
        parent: Overlay.overlay //! Parent must be Overlay.overlay so MouseArea works when there is a Popup opened
        preventStealing: true
        z: 10
        onPressed: function(event) {
            if (_screenSaver.visible) {
                _screenSaver.close();
            } else {
                //! Restart timer
                _screenSaverTimer.restart();
            }

            //! Set accepted to true to let it propagate to below items
            event.accepted = false;
        }
    }

    Timer {
        id: _screenSaverTimer
        interval: 5000
        running: !_screenSaver.visible
        repeat: false
        onTriggered: {
            _screenSaver.open();
        }
    }
}

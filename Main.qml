import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.VirtualKeyboard

import Ronia
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
    width: AppStyle.size
    height: AppStyle.size

    visible: true
//    visibility: Window.FullScreen
    title: qsTr("Template" + "               " + currentFile)

    //! Create defualt repo and root object to save and load
    Component.onCompleted: {
        AppCore.defaultRepo.initRootObject("QSObject");

        //! Refereh wifis.
        NetworkInterface.refereshWifis();
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
    Material.background: AppStyle.backgroundColor
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

    Flickable {
        width: window.width
        height: window.height - (window.height - _virtualKb.y)
        interactive: _virtualKb.active
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: window.width

        MainView {
            id: mainView
            anchors.fill: parent
            uiSession: window.uiSession
        }
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
        device: uiSession.appModel
        uiPreference: uiSession?.uiPreferences
    }

    //! This mouse area is to detect app interactions to prevent screen saver being shown
    MouseArea {
        anchors.fill: parent
        parent: Overlay.overlay //! Parent must be Overlay.overlay so MouseArea works when there is a Popup opened
        propagateComposedEvents: true
        preventStealing: false
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
        interval: 20000
        running: !_screenSaver.visible
        repeat: false
        onTriggered: {
            _screenSaver.open();
        }
    }

    //! A Timer to periodically refresh wifis (every 2 seconds)
    Timer {
        running: true
        interval: 5000
        repeat: true
        onTriggered: {
            NetworkInterface.refereshWifis();
        }
    }

    //! Virtual keyboard
    InputPanel {
        id: _virtualKb
        z: 99
        x: 0
        y: window.height
        width: window.width
        implicitHeight: keyboard.height + _closeBtn.height

        //! ToolButton to close keyboard.
        //! This SHOULD steal focus from other controls, otherwise an already focused TextField
        //! will not be able to activate keyboard. No onClicked handling is required.
        TabButton {
            id: _closeBtn
            anchors {
                right: parent.right
                rightMargin: 4
            }
            padding: 8
            background: null
            contentItem: RoniaTextIcon {
                anchors.centerIn: parent
                text: "\uf078"
            }
        }

        states: State {
            name: "visible"
            when: _virtualKb.active
            PropertyChanges {
                target: _virtualKb
                y: window.height - _virtualKb.height
            }
        }

        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true

            ParallelAnimation {
                NumberAnimation {
                    properties: "y"
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}

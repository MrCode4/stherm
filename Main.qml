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

    //! Save the app configuration when the app closed
    onClosing: {
        // Save to found path
        AppCore.defaultRepo.saveToFile(window.uiSession.configFilePath);
    }

    //! Create defualt repo and root object to save and load
    Component.onCompleted: {

        // Create and prepare DefaultRepo and RootModel as root.
        AppCore.defaultRepo = AppCore.createDefaultRepo(["QtQuickStream", "Stherm"]);

        // Bind appModel to qsRootObject to capture loaded model from configuration.
        window.uiSession.appModel = Qt.binding(function() { return AppCore.defaultRepo.qsRootObject;});

        // Load the file
        console.info("Load the config file: ", window.uiSession.configFilePath)
        if (AppCore.defaultRepo.loadFromFile(window.uiSession.configFilePath))
            console.info("Config file succesfully loaded.")
        else
            AppCore.defaultRepo.initRootObject("SimDevice");

        //! set screen saver timeout here. default is 20000
        ScreenSaverManager.screenSaverTimeout = 20000;

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
    Material.background: Style.background
    Material.accent: Style.accent

    /* Children
     * ****************************************************************************************/

    StackLayout {
        id: _normalAndVacationModeStV
        currentIndex: uiSession?.appModel.systemMode === I_Device.SystemMode.Vacation ? 1 : 0

        Flickable {
            id: _mainViewFlick
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

        VacationModeView {
            id: _vacationModeView
            uiSession: window.uiSession
            visible: parent.currentIndex === 1
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
        visible: ScreenSaverManager.state === ScreenSaverManager.Timeout
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

    //! MessagePopupView
    MessagePopupView {
        messageController: uiSession?.messageController ?? null
    }

    //! This Timer is used to generate arbitrary Messages (alert or notification)
    Timer {
        interval: 8000
        repeat: true
        running: true
        onTriggered: {
            if (Math.random() > 0.5) {
                //! Create an alert
                var now = new Date();
                if (Math.random() > 0.5) {
                    //! Create an Alert
                    uiSession.messageController.addNewMessageFromData(
                                Message.Type.Alert,
                                "Arbitrary Alert Number " + Math.floor(Math.random() * 100),
                                now.toLocaleTimeString("MMMM dd ddd - hh:mm:ss"))
                } else {
                    //! Create a Notification
                    uiSession.messageController.addNewMessageFromData(
                                Message.Type.Notification,
                                "Arbitrary Notification Number " + Math.floor(Math.random() * 100),
                                now.toLocaleTimeString("MMMM dd ddd - hh:mm:ss"))
                }
            }
        }
    }
}

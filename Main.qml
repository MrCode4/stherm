import QtQuick
import QtQuick.Window
import QtQuick.Layouts
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
    property UiSession  uiSession: UiSession {
        popupLayout: popUpLayoutId
    }

    property string     currentFile: uiSession.currentFile

    /* Object Properties
     * ****************************************************************************************/
    x: 0
    y: 0
    width: AppStyle.size
    height: AppStyle.size

    visible: false
    title: qsTr("STherm")

    //! Save the app configuration when the app closed
    onClosing: {
        // Save to found path
        AppCore.defaultRepo.saveToFile(window.uiSession.configFilePath);
    }

    onVisibleChanged: {
        if (visible) {
            _splashLoader.sourceComponent = null;
        }
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
            AppCore.defaultRepo.initRootObject("Device");

        //! set screen saver timeout here. default is 20000
        ScreenSaverManager.screenSaverTimeout = 20000;
    }

    /* Fonts
     * ****************************************************************************************/
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Thin-100.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Solid-900.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Regular-400.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Light-300.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/RobotoMono-Regular.ttf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Montserrat-Regular.ttf" }


    /* Style
     * ****************************************************************************************/
    Material.theme: Material.Dark
    Material.background: Style.background
    Material.accent: Style.accent

    /* Children
     * ****************************************************************************************/

    StackLayout {
        id: _normalAndVacationModeStV
        currentIndex: uiSession?.appModel.systemMode === AppSpec.SystemMode.Vacation ? 1 : 0

        Flickable {
            id: _mainViewFlick
            width: window.width
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: width
            contentHeight: window.width

            MainView {
                id: mainView
                anchors.fill: parent
                uiSession: window.uiSession
            }

            Behavior on contentY {
                NumberAnimation { }
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
        id: popUpLayoutId
        anchors.fill: parent
    }

    ShortcutManager {
        uiSession: window.uiSession
    }

    //! ScreenSaver
    ScreenSaver {
        id: _screenSaver
        anchors.centerIn: parent
        visible: ScreenSaverManager.state === ScreenSaverManager.Timeout
        deviceController: uiSession.deviceController
        device: uiSession.appModel
    }

    //! A Timer to periodically refresh wifis (every 20 seconds); First refresh wifis after 1
    //! seconds and then refresh every 20 seconds
    Timer {
        running: uiSession.refreshWifiEnabled
        interval: 1000
        repeat: true
        onTriggered: {
            NetworkInterface.refereshWifis();

            if (interval < 20000) {
                interval = 20000;
            }
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

        onActiveChanged: {
            if (active && (window.activeFocusControl instanceof TextInput
                           || window.activeFocusControl instanceof TextEdit)) {
                var activeControlPos = window.activeFocusControl.mapToItem(window.contentItem, 0, 0);

                //! Move active control to the center of area above keyboard
                var targetY = (window.height
                               - _virtualKb.implicitHeight
                               - window.activeFocusControl.height) / 2
                _mainViewFlick.contentY = activeControlPos.y - targetY;
                state = "visible";
            } else {
                _mainViewFlick.contentY = 0;
                state = "invisible";
            }
        }

        Component.onCompleted: {
            //! Increase key height and keyboard height
            keyboard.style.keyboardDesignHeight = keyboard.style.keyboardDesignHeight * 1.3
            keyboard.style.keyboardHeight = keyboard.style.keyboardDesignHeight / 3.8;
        }

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

        states: [
            State {
                name: "invisible"

                PropertyChanges {
                    target: _virtualKb
                    y: window.height
                }

                PropertyChanges {
                    target: _mainViewFlick
                    interactive: false
                    height: window.height
                }
            },

            State {
                name: "visible"

                PropertyChanges {
                    target: _virtualKb
                    y: window.height - _virtualKb.height
                }

                PropertyChanges {
                    target: _mainViewFlick
                    interactive: true
                    height: window.height - _virtualKb.implicitHeight
                }
            }
        ]

        transitions: [
            Transition {
                from: "*"
                to: "visible"

                SequentialAnimation {
                    PropertyAnimation {
                        target: _mainViewFlick
                        property: "interactive"
                        duration: 0
                    }

                    ParallelAnimation {
                        NumberAnimation {
                            target: _mainViewFlick
                            property: "height"
                            duration: 300
                        }

                        NumberAnimation {
                            properties: "y"
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            },
            Transition {
                from: "*"
                to: "invisible"

                SequentialAnimation {
                    ParallelAnimation {
                        NumberAnimation {
                            target: _mainViewFlick
                            property: "height"
                            duration: 300
                        }

                        NumberAnimation {
                            properties: "y"
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }

                    PropertyAnimation {
                        target: _mainViewFlick
                        property: "interactive"
                        duration: 0
                    }
                }
            }
        ]
    }

    //! SplashScreen Loader
    Loader {
        id: _splashLoader
        sourceComponent: _splashCompo
        onLoaded: {
            item.visible = true;
        }
    }

    Component {
        id: _splashCompo

        SplashScreen {
            Material.theme: window.Material.theme
            Material.foreground: window.Material.foreground
            x: 0
            y: 0
            width: window.width
            height: window.height
            visible: true

            onReady: {
                window.visible = true;
            }
        }
    }

    //! MessagePopupView
    MessagePopupView {
        uiSession: window.uiSession
        messageController: uiSession?.messageController ?? null
    }

    //! This Timer is used to generate arbitrary Messages (alert or notification)
    Timer {
        interval: 8000
        repeat: true
        running: true
        onTriggered: {
            if (Math.random() > 0.98) {
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

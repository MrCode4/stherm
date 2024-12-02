import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.VirtualKeyboard
import QtQuick.VirtualKeyboard.Styles
import QtQuick.VirtualKeyboard.Settings

import Ronia
import Stherm
import QtQuickStream

/*! ***********************************************************************************************
 * This is the highest level graphical object, i.e., the main application window. The state
 * of each instance is stored in the UiSession, which needs to be passed to its children.
 * Multiple instances can be created.
 *
 * ************************************************************************************************/
ApplicationWindow {
    id: window

    property string     currentFile: uiSessionId.currentFile

    //! deviceControllerCPP: Use in initialization of app
    property DeviceControllerCPP deviceControllerCPP: uiSessionId.deviceController.deviceControllerCPP

    /* Object Properties
     * ****************************************************************************************/
    x: 10
    y: 6
    width: AppStyle.size - 2 * x
    height: AppStyle.size - 2 * y

    visible: false
    title: qsTr("STherm")

    //! Save the app configuration when the app closed
    onClosing: {
        // Save to found path
        uiSessionId.deviceController.saveSettings();
    }

    onVisibleChanged: {
        if (visible) {
            _splashLoader.sourceComponent = null;
        }
    }

    //! Create default repo and root object to save and load
    Component.onCompleted: {

        // Create and prepare DefaultRepo and RootModel as root.
        AppCore.defaultRepo = AppCore.createDefaultRepo(["QtQuickStream", "Stherm"]);

        // Bind appModel to qsRootObject to capture loaded model from configuration.
        uiSessionId.appModel = Qt.binding(function() { return AppCore.defaultRepo.qsRootObject;});

        // Remove saved files after restart and update the app and get settings from server
        // to fix any errors that may have occurred after updating the app.
        // TODO we need a way to detect if we should remove the file
        if (deviceControllerCPP.checkUpdateMode() && false) {
            QSFileIO.removeFile(uiSessionId.configFilePath);
            QSFileIO.removeFile(uiSessionId.recoveryConfigFilePath);
            QSFileIO.removeFile("sthermConfig.QQS.json");

            // Load app with defaults
            console.info("Load the app with default settings");
            AppCore.defaultRepo.initRootObject("Device");

            uiSessionId.currentFile = "Update Device";

            // Update setting with server
            deviceControllerCPP.system.fetchSettings();

        } else {
            // Load model from the file after initialize setup, normal restart, etc...

            // Load the file
            // check if not exist uiSessionId.configFilePath
            // then load from relative path (sthermConfig.QSS.json)), and remove it
            if (AppCore.defaultRepo.loadFromFile(uiSessionId.configFilePath)) {
                console.info("Load the config file: ", uiSessionId.configFilePath);
                console.info("Config file succesfully loaded.");
                uiSessionId.currentFile = uiSessionId.configFilePath;

            } else if (AppCore.defaultRepo.loadFromFile("sthermConfig.QQS.json")) {
                console.info("Load the config file: sthermConfig.QQS.json");
                console.info("old Config file succesfully loaded.");
                uiSessionId.currentFile = "sthermConfig.QQS.json";

            } else if (AppCore.defaultRepo.loadFromFile(uiSessionId.recoveryConfigFilePath)) {
                console.info("Load the config file:", uiSessionId.recoveryConfigFilePath);
                console.info("recovery Config file succesfully loaded.");
                uiSessionId.currentFile = uiSessionId.recoveryConfigFilePath;

            } else {
                AppCore.defaultRepo.initRootObject("Device");
                uiSessionId.currentFile = "Device";
            }

            // Remove the relative file from the directory.
            QSFileIO.removeFile("sthermConfig.QQS.json");

            // if any load was successful, write it to recovery
            // defaults also saved.
            console.log("Save recovery file: ", AppCore.defaultRepo.saveToFile(uiSessionId.recoveryConfigFilePath));
        }



        //! Load DST effect and then current timezone to DateTimeManager
        //! NOTE: Order of setting effect DST and current timezone is important.
        //! effectDst waits for the other to be finished, so it should be last
        if (uiSessionId.appModel.setting.currentTimezone !== "") {
            DateTimeManager.currentTimeZone = uiSessionId.appModel.setting.currentTimezone;
        }
        DateTimeManager.effectDst = uiSessionId.appModel.setting.effectDst;

        //! set screen saver timeout here. default is 20000
        ScreenSaverManager.screenSaverTimeout = 30000;
    }

    /* Fonts
     * ****************************************************************************************/
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Thin-100.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Solid-900.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Regular-400.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Font Awesome 6 Pro-Light-300.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/unifont.otf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Roboto-Regular.ttf" }
    FontLoader { source: "qrc:/Stherm/Fonts/RobotoMono-Regular.ttf" }
    FontLoader { source: "qrc:/Stherm/Fonts/Montserrat-Regular.ttf" }


    /* Style
     * ****************************************************************************************/
    Material.theme: Material.Dark
    Material.background: Style.background
    Material.accent: Style.accent

    /* Children
     * ****************************************************************************************/
    //! This Connections are used to sync current timezone and dst in DateTimeManager and Setting
    Connections {
        target: DateTimeManager

        function onEffectDstChanged()
        {
            if (uiSessionId.appModel.setting.effectDst !== DateTimeManager.effectDst) {
                uiSessionId.appModel.setting.effectDst = DateTimeManager.effectDst;
            }
        }

        function onCurrentTimeZoneChanged()
        {
            if (uiSessionId.appModel.setting.currentTimezone !== DateTimeManager.currentTimeZone.id) {
               uiSessionId.appModel.setting.currentTimezone = DateTimeManager.currentTimeZone.id;
            }
        }
    }

    Connections {
        target: uiSessionId.appModel.setting
        function onCurrentTimezoneChanged() {

            if (uiSessionId.appModel.setting.currentTimezone !== DateTimeManager.currentTimeZone.id) {
                DateTimeManager.currentTimeZone = uiSessionId.appModel.setting.currentTimezone;
            }
        }

        function onEffectDstChanged() {

            if (uiSessionId.appModel.setting.effectDst !== DateTimeManager.effectDst) {
                DateTimeManager.effectDst = uiSessionId.appModel.setting.effectDst;
            }
        }
    }

    Connections {
        target: deviceControllerCPP.system
        function onAreSettingsFetchedChanged(success) {
            uiSessionId.settingsReady = success;
        }
    }

    UiSession {
        id: uiSessionId
        popupLayout: popUpLayoutId
        toastManager:toastManagerId
        onIsAnyPopupVisibleChanged: window.postponeOrResumePerfTest()
    }

    StackLayout {
        id: _normalAndVacationModeStV
        currentIndex: uiSessionId.appModel.systemSetup.isVacation ? 1 : (uiSessionId.showMainWindow ? 0 : 1)

        Flickable {
            id: _mainViewFlick

            ScrollIndicator.vertical: ScrollIndicator { }

            width: window.width
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: width
            contentHeight: window.width

            MainView {
                id: mainView
                anchors.fill: parent
                uiSession: uiSessionId
                onStackViewDepthChanged: window.postponeOrResumePerfTest()
            }

            Behavior on contentY {
                NumberAnimation { }
            }
        }

        VacationModeView {
            id: _vacationModeView
            uiSession: uiSessionId
            visible: parent.currentIndex === 1
        }
    }

    //! Popup layout
    PopUpLayout {
        id: popUpLayoutId
        anchors.fill: parent
        mandatoryUpdate: uiSessionId.deviceController.mandatoryUpdate
    }

    ShortcutManager {
        uiSession: uiSessionId
    }

    //! ScreenSaver
    ScreenSaver {
        id: _screenSaver
        anchors.centerIn: parent
        visible: ScreenSaverManager.state === ScreenSaverManager.Timeout
        uiSession: uiSessionId
        onOpened: {
            uiSessionId.showHome();
            window.postponeOrResumePerfTest();
        }

        onClosed: {
            if (deviceController.initialSetupNoWIFI) {
                uiSession.popUps.showLimitedInitialSetupPopup();
            }
        }
    }

    PerfTestPopup {
        uiSession: uiSessionId
        z: _screenSaver.z + 1
        visible: PerfTestService.state >= PerfTestService.Warmup &&
                 (uiSession.appModel.lock.isLock == false || _screenSaver.visible)
    }

    function postponeOrResumePerfTest() {
        console.log('Stack-View-Depth-and-Popup', _screenSaver.visible, uiSessionId.isAnyPopupVisible, mainView.stackViewDepth);
        if (_screenSaver.opened == false && (mainView.stackViewDepth > 1 || uiSessionId.isAnyPopupVisible)) {
            let message = mainView.stackViewDepth > 1 ?
                    "There are other views active on top of Home"
                  : "There are popup active for the user to handle";
            PerfTestService.postponeTest(message);
        }
        else {
            PerfTestService.resumeTest();
        }
    }

    //! A Timer to periodically refresh wifis (every 20 seconds); First refresh wifis after 1
    //! seconds and then refresh every 20 seconds
    Timer {
        running: uiSessionId.refreshWifiEnabled
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
            if (active && !(window.activeFocusControl instanceof PINTextField) &&
                    (window.activeFocusControl instanceof TextInput
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
            VirtualKeyboardSettings.locale = "en_US";

            //! Documents in KeyboardStyle QML Type
            keyboard.style.keyPanel = keyboardKeyCompo;
            keyboard.style.spaceKeyPanel = spaceStyleCompo;
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

    //! Component for keyboard key. This will be set in keyboard style
    Component {
        id: keyboardKeyCompo

        KeyPanel {
            id: keypanel

             Rectangle {
                 anchors.fill: parent
                 anchors.margins: 2
                 color: [".", ","].includes(keypanel.control.displayText) ? "#4F5B62" : Style.background;
                 radius: 4

                 Text {
                     anchors.centerIn: parent
                     font.family: Application.font.family
                     font.capitalization: keypanel.control.uppercased ? "AllUppercase" : "MixedCase"
                     color: Qt.darker(Style.foreground, keypanel.control.pressed ? 2 : 1)
                     text: keypanel.control.displayText
                 }
             }
         }
    }

    //! Style of space key in virtual keyboard
    Component {
        id: spaceStyleCompo

        KeyPanel {
            id: keypanel

            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                color: Qt.darker("#4F5B62", keypanel.control.pressed ? 1.1 : 1);
                radius: 4

                Text {
                    anchors.centerIn: parent
                    font.family: Application.font.family
                    font.pixelSize: Application.font.pixelSize * 0.8
                    font.capitalization: keypanel.control.uppercased ? Font.AllUppercase : Font.MixedCase
                    color: Qt.darker(Style.foreground, keypanel.control.pressed ? 1.5 : 1)
                    text: "English"
                }
            }
        }
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
        uiSession: uiSessionId
    }

    //ToastManager component for managing and displaying registered toasts
    ToastManager{
        id: toastManagerId
        toastComponent:toastViewId
    }

    //Toast view for displaying toast messages
    Toast{
        id:toastViewId
        x: (parent.width - width) / 2
        y: parent.height - height - 16
    }
}

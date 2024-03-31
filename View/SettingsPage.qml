import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SettingsPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Setting
    property Setting    setting: uiSession?.appModel?.setting ?? null

    //!‌ This timer is used to apply settings to device lively
    property Timer onlineTimer: Timer {
        repeat: false
        running: false
        interval: 50
        onTriggered: applyToModel()

        function startTimer()
        {
            if (!onlineTimer.running) {
                start();
            }
        }
    }


    /* Object properties
     * ****************************************************************************************/
    title: "Settings"
    leftPadding: 10 * scaleFactor
    backButtonCallback: function() {
        //! Check if color is modified
        var selectedTempUnit = _tempCelciUnitBtn.checked ? AppSpec.TempratureUnit.Cel : AppSpec.TempratureUnit.Fah;

        if (setting && (setting.brightness !== _brightnessSlider.value
                        || setting.adaptiveBrightness !== _adaptiveBrSw.checked
                        || setting.volume !== _speakerSlider.value
                        || setting.tempratureUnit !== selectedTempUnit)) {
            //! This means that changes are occured that are not saved into model
            uiSession.popUps.exitConfirmPopup.accepted.connect(confirmtBtn.clicked);
            uiSession.popUps.exitConfirmPopup.rejected.connect(goBack);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.exitConfirmPopup);
        } else {
            goBack();
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        id: confirmtBtn
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            applyToModel();

            deviceController.finalizeSettings();

            //! Make a copy of last applied data to Setting
            makeCopyOfSettings();

            goBack();
        }
    }

    QtObject {
        id: internal

        //! This property will hold last applied data to Setting
        property var copyOfSettings: ({})
    }

    Flickable {
        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: _root.contentItem.y
            parent: _root
            height: _root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: _contentCol.implicitHeight

        ColumnLayout {
            id: _contentCol
            width: parent.width
            spacing: 8

            Label {
                opacity: 0.6
                text: "Brightness"
            }

            //! Brightness row
            RowLayout {
                spacing: 8

                RoniaTextIcon {
                    Layout.rightMargin: 16
                    text: "\ue0c9" //! brightness icon
                }

                Label {
                    opacity: 0.6
                    font.pointSize: _root.font.pointSize * 0.9
                    text: _brightnessSlider.from.toLocaleString(locale, "f", 0)
                }

                Slider {
                    id: _brightnessSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: appModel?.setting?.brightness ?? 0

                    onValueChanged: onlineTimer.startTimer();
                }

                Label {
                    opacity: 0.6
                    font.pointSize: _root.font.pointSize * 0.9
                    text: _brightnessSlider.to.toLocaleString(locale, "f", 0)
                }
            }

            Label {
                opacity: 0.6
                Layout.topMargin: 12
                text: "Speaker"
            }

            //! Speaker row
            RowLayout {
                spacing: 8

                RoniaTextIcon {
                    Layout.rightMargin: 16
                    text: "\uf6a8" //! volume icon
                }

                Label {
                    opacity: 0.6
                    font.pointSize: _root.font.pointSize * 0.9
                    text: _speakerSlider.from.toLocaleString(locale, "f", 0)
                }

                Slider {
                    id: _speakerSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: appModel?.setting?.volume ?? 0
                }

                Label {
                    opacity: 0.6
                    font.pointSize: _root.font.pointSize * 0.9
                    text: _speakerSlider.to.toLocaleString(locale, "f", 0)
                }
            }

            //! Adaptive brightness
            RowLayout {
                Layout.topMargin: 12

                Label {
                    opacity: 0.6
                    Layout.fillWidth: true
                    text: "Adaptive Brightness"
                }

                Switch {
                    id: _adaptiveBrSw
                    checked: appModel?.setting?.adaptiveBrightness ?? false

                    onCheckedChanged: {
                        onlineTimer.startTimer()
                    }
                }
            }

            //! Temprature unit
            GridLayout {
                Layout.topMargin: 12
                columnSpacing: 8
                rowSpacing: 8
                columns: 3

                Label {
                    opacity: 0.6
                    Layout.fillWidth: true
                    text: "Temprature"
                }

                RadioButton {
                    id: _tempFarenUnitBtn
                    text: "\u00b0F"
                    checked: appModel?.setting?.tempratureUnit === AppSpec.TempratureUnit.Fah
                }

                RadioButton {
                    id: _tempCelciUnitBtn
                    text: "\u00b0C"
                    checked: appModel?.setting?.tempratureUnit === AppSpec.TempratureUnit.Cel
                }
            }

            //! Reset setting Button
            ButtonInverted {
                text: "Reset"
                onClicked: {
                    if (uiSession) {
                        uiSession.popupLayout.displayPopUp(_resetSettings, true);
                    }
                }
            }
        }
    }

    //! Save settings
    function applyToModel() {
        if (deviceController) {
            deviceController.setSettings(_brightnessSlider.value,
                                         _speakerSlider.value,
                                         _tempFarenUnitBtn.checked ? AppSpec.TempratureUnit.Fah
                                                                   : AppSpec.TempratureUnit.Cel,
                                         _adaptiveBrSw.checked);
        }
    }

    function makeCopyOfSettings()
    {
        if (setting) {
            internal.copyOfSettings["brightness"]           = setting.brightness;
            internal.copyOfSettings["adaptiveBrightness"]   = setting.adaptiveBrightness;
            internal.copyOfSettings["volume"]               = setting.volume;
            internal.copyOfSettings["tempratureUnit"]       = setting.tempratureUnit;
        }
    }

    //! This method is used to go back
    function goBack()
    {
        uiSession.popUps.exitConfirmPopup.accepted.disconnect(confirmtBtn.clicked);
        uiSession.popUps.exitConfirmPopup.rejected.disconnect(goBack);

        if (_root.StackView.view) {
            //! Then Page is inside an StackView
            if (_root.StackView.view.currentItem == _root) {
                _root.StackView.view.pop();
            }
        }
    }

    //! Reset settings pop
    ResetSettingsDialog {
        id: _resetSettings

        onAccepted: {
            //! Perform reseting settings
            if (deviceController) {
                deviceController.setSettings(80,
                                             80,
                                             AppSpec.TempratureUnit.Cel,
                                             true);
            }
        }
    }

    Component.onCompleted: {
        deviceController.updateEditMode(AppSpec.EMSettings);

        makeCopyOfSettings();
    }

    Component.onDestruction: {
        deviceController.updateEditMode(AppSpec.EMNone);

        if (setting) {
            if (setting.brightness !== internal.copyOfSettings.brightness
                    || setting.adaptiveBrightness !== internal.copyOfSettings.adaptiveBrightness
                    || setting.volume !== internal.copyOfSettings.volume
                    || setting.tempratureUnit !== internal.copyOfSettings.tempratureUnit) {
                //! Reset to last saved setting
                deviceController.setSettings(
                            internal.copyOfSettings.brightness,
                            internal.copyOfSettings.volume,
                            internal.copyOfSettings.tempratureUnit,
                            internal.copyOfSettings.adaptiveBrightness
                            );
            }
        }
    }
}

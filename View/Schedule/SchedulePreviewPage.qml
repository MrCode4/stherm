import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SchedulePreviewPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Schedules controlller
    property SchedulesController    schedulesController: uiSession?.schedulesController ?? null

    //! Schedule
    property ScheduleCPP            schedule

    //! Can schedule fields be editabled
    property bool                   isEditable: false

    //!
    readonly property ScheduleCPP   scheduleToDisplay: isEditable ? internal.scheduleToEdit : schedule

    property int temperatureUnit:      appModel?.setting?.tempratureUnit ?? AppSpec.defaultTemperatureUnit

    //! Minimum temprature
    property real               minTemperature: deviceController?.getMinValue(schedule?.systemMode, temperatureUnit) ?? 40

    //! Maximum temprature
    property real               maxTemperature: deviceController?.getMaxValue(schedule?.systemMode, temperatureUnit) ?? 90


    /* Signals
     * ****************************************************************************************/
    signal done();

    /* Object properties
     * ****************************************************************************************/
    rightPadding: 4
    title: "Schedule Preview"
    backButtonVisible: false
    titleHeadeingLevel: 4
    backButtonCallback: function() {
        //! Check if schedule is edited
        if (isEditable) {
            if (internal.isScheduleModified()) {
                //! Prompt user for save confirm
                confPop.message = "Schedule is modified, do you want to save it before exiting?";
                confPop.detailMessage = "Changes are lost if No is pressed";
                confPop.acceptCallback = saveOnBackProcedure;
                confPop.rejected.connect(this, goBack)

                confPop.open();
                return;
            }
        }
        done();
        goBack();
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm change button
    ToolButton {
        id: confirmtBtn
        visible: internal.isScheduleModified()
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! First check if start and end time are correct
            internal.exitAfterSave = false; //! Do not exit after save
            if (checkValidity()) {
                if (!checkOverlappings()) {
                    promptSavingSchedule(saveSchedule);
                }
            }
        }

        function promptSavingSchedule(acceptCallback=saveSchedule)
        {
            //! Prompt user for save confirm
            //! It's might be better to show this only if this schedule is active since changing its
            //! properties may make it inactive
            confPop.message = "Are you sure you want to save changes to schedule?"
            confPop.acceptText = qsTr("Yes");
            confPop.rejectText = qsTr("No");
            confPop.acceptCallback = acceptCallback;

            confPop.open();
        }
    }

    Flickable {
        id: itemsFlickable

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: _root.contentItem.y
            parent: _root
            height: _root.contentItem.height - 16
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: _previewContentLay.implicitHeight
        contentWidth: width

        ColumnLayout {
            id: _previewContentLay
            anchors.fill: parent

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Name"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: scheduleToDisplay?.name ?? ""
                        elide: "ElideRight"
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleNamePage.qml", {
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "uiSession": uiSession,
                                                      "schedule": internal.scheduleToEdit
                                                  });
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Type"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: AppSpec.scheduleTypeNames[scheduleToDisplay?.type ?? AppSpec.Away]
                        elide: "ElideRight"
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTypePage.qml", {
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "uiSession": uiSession,
                                                      "schedule": internal.scheduleToEdit
                                                  });
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight
                Layout.leftMargin: 8
                Layout.rightMargin: 8

                RowLayout {
                    anchors.fill: parent
                    spacing: 16

                    Label {
                        // Layout.fillWidth: true
                        font.bold: true
                        text: "Mode"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Qt.AlignRight
                        elide: Text.ElideRight
                        text: {
                            if (scheduleToDisplay.systemMode === AppSpec.Cooling) {
                                return "Cooling";

                            } else if (scheduleToDisplay.systemMode === AppSpec.Heating || scheduleToDisplay.systemMode === AppSpec.EmergencyHeat) {
                                return "Heating";
                            }

                            return "Auto";
                        }
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Temperature"
                    }

                    Label {
                        readonly property string unit: AppSpec.temperatureUnitString(appModel?.setting?.tempratureUnit)

                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: {
                            if (scheduleToDisplay.systemMode === AppSpec.Heating || scheduleToDisplay.systemMode === AppSpec.EmergencyHeat) {
                                // Show the minimum temperature
                                var value =  Utils.convertedTemperatureClamped(scheduleToDisplay?.minimumTemperature ?? 10, appModel?.setting?.tempratureUnit, minTemperature, maxTemperature);
                                value = AppUtilities.getTruncatedvalue(value);

                                return Number(value).toLocaleString(locale, "f", 0) + ` \u00b0${unit}`;

                            } else if (scheduleToDisplay.systemMode === AppSpec.Cooling) {
                                // Show the maximum temperature
                                var value = Utils.convertedTemperatureClamped(scheduleToDisplay?.maximumTemperature ?? 0, appModel?.setting?.tempratureUnit, minTemperature, maxTemperature);
                                value = AppUtilities.getTruncatedvalue(value);
                                return Number(value).toLocaleString(locale, "f", 0)
                                        + ` \u00b0${unit}`;

                            } else {

                                var minValue = Utils.convertedTemperatureClamped(scheduleToDisplay?.minimumTemperature ?? 10, appModel?.setting?.tempratureUnit, minTemperature, maxTemperature);
                                minValue = AppUtilities.getTruncatedvalue(minValue);

                                var maxValue = Utils.convertedTemperatureClamped(scheduleToDisplay?.maximumTemperature ?? 0, appModel?.setting?.tempratureUnit, minTemperature, maxTemperature);
                                maxValue = AppUtilities.getTruncatedvalue(maxValue);

                                // Show the maximum and minimum temperature values.
                                return Number(minValue).toLocaleString(locale, "f", 0) + " - " +
                                        Number(maxValue).toLocaleString(locale, "f", 0)
                                        + ` \u00b0${unit}`
                            }
                        }
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTempraturePage.qml", {
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "uiSession": uiSession,
                                                      "schedule": internal.scheduleToEdit,
                                                      "editMode": true
                                                  });
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                // TODO hide Humudity:page when accessories is None.
                // visible: false

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Humidity"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: `${scheduleToDisplay?.humidity ?? 0} %`
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleHumidityPage.qml", {
                                                      "uiSession": uiSession,
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "schedule": internal.scheduleToEdit,
                                                      "editMode": true
                                                  });
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Start Time"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: scheduleToDisplay?.startTime ?? ""
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTimePage.qml", {
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "uiSession": uiSession,
                                                      "schedule": internal.scheduleToEdit,
                                                      "editMode": true
                                                  });
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "End Time"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: scheduleToDisplay?.endTime ?? ""
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTimePage.qml", {
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "uiSession": uiSession,
                                                      "schedule": internal.scheduleToEdit,
                                                      "editMode": true
                                                  });
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Repeats"
                    }

                    RowLayout {
                        Layout.fillWidth: false
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignRight

                        Repeater {
                            model: scheduleToDisplay?.repeats.length > 0 ? scheduleToDisplay.repeats.split(",") : ["No repeat"]
                            delegate: Label {
                                Layout.alignment: Qt.AlignTop
                                text: modelData

                                Rectangle {
                                    anchors {
                                        top: parent.bottom
                                        horizontalCenter: parent.horizontalCenter
                                    }
                                    width: 4
                                    height: 4
                                    radius: 2
                                }
                            }
                        }
                    }
                }

                onClicked: {
                    if (!isEditable) return;

                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleRepeatPage.qml", {
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "uiSession": uiSession,
                                                      "schedule": internal.scheduleToEdit,
                                                      "editMode": true
                                                  });
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Data Source"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: scheduleToDisplay?.dataSource ?? ""
                        elide: "ElideRight"
                    }
                }

                onClicked: {
                    if (!isEditable) return;

                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleDataSourcePage.qml", {
                                                      "backButtonVisible": _root.backButtonVisible,
                                                      "uiSession": uiSession,
                                                      "schedule": internal.scheduleToEdit
                                                  });
                    }
                }
            }
        }
    }

    ConfirmPopup {
        id: confPop

        property var acceptCallback

        onAccepted: if (acceptCallback instanceof Function) acceptCallback();
        onClosed: acceptCallback = null;
    }

    onIsEditableChanged: if (isEditable) internal.copySchedule()
    onScheduleChanged: if (isEditable) internal.copySchedule()

    QtObject {
        id: internal

        //! This boolean holds if it should exit this page after save happens, i.e in saveSchedule()
        property bool exitAfterSave: false

        //! A copy of _root.schedule in edit mode so user can preview changes and confirm before
        //! saving changes to the original schedule.
        property ScheduleCPP scheduleToEdit: ScheduleCPP {
            systemMode: deviceController.device.systemSetup.systemMode
        }

        //! Overlapping schedules
        property var         overlappingSchedules: []

        //! Get a copy of _root.schedule to be edited
        function copySchedule()
        {
            if (_root.schedule) {
                scheduleToEdit.enable = false; //! This is always false so overlapping check dont take
                //! place in ScheduleTimePage since this copy will
                //! overlap with itself
                scheduleToEdit.name = _root.schedule.name;
                scheduleToEdit.type = _root.schedule.type;
                scheduleToEdit.minimumTemperature = _root.schedule.minimumTemperature;
                scheduleToEdit.maximumTemperature = _root.schedule.maximumTemperature;
                scheduleToEdit.humidity = _root.schedule.humidity;
                scheduleToEdit.startTime = _root.schedule.startTime;
                scheduleToEdit.endTime = _root.schedule.endTime;
                scheduleToEdit.repeats = _root.schedule.repeats;
                scheduleToEdit.dataSource = _root.schedule.dataSource;
                scheduleToEdit.systemMode = _root.schedule.systemMode;
            }
        }

        //! Check if copied schedule is modified and needs to be saved
        function isScheduleModified()
        {
            return isEditable && _root.schedule && internal.scheduleToEdit
                    ? _root.schedule.name !== internal.scheduleToEdit.name
                      || _root.schedule.type !== internal.scheduleToEdit.type
                      || _root.schedule.minimumTemperature !== internal.scheduleToEdit.minimumTemperature
                      || _root.schedule.maximumTemperature !== internal.scheduleToEdit.maximumTemperature
                      || _root.schedule.humidity !== internal.scheduleToEdit.humidity
                      || _root.schedule.startTime !== internal.scheduleToEdit.startTime
                      || _root.schedule.endTime !== internal.scheduleToEdit.endTime
                      || _root.schedule.repeats !== internal.scheduleToEdit.repeats
                      || _root.schedule.dataSource !== internal.scheduleToEdit.dataSource
                    : false
        }

        //! Makes editing schedule saved and enables it, also disables overlapping schedules
        function saveEnabledSchedule()
        {
            //! Disable overlapping schedules if any.
            overlappingSchedules.forEach((element, index) => {
                                             element.enable = false;
                                             schedulesController.editScheduleInServer(element);
                                         });
            //! Enable this.
            schedule.enable = true;

            //! Perform saving (and disconnecting from ScheduleOverlapPopup's signal
            saveScheduleAndDisconnectScheduleOverlapPopup();
        }

        //! Makes editing schedule saved and disabled
        function saveDisabledSchedule()
        {
            //! Disable this.
            schedule.enable = false;

            //! Perform saving (and disconnecting from ScheduleOverlapPopup's signal
            saveScheduleAndDisconnectScheduleOverlapPopup()
        }

        //! Disconnect from ScheduleOverlapPopup's signals and prompts for saving schedule
        function saveScheduleAndDisconnectScheduleOverlapPopup()
        {
            uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(saveEnabledSchedule);
            uiSession.popUps.scheduleOverlapPopup.rejected.disconnect(saveDisabledSchedule);

            saveSchedule();
        }
    }

    /* Methods
     * ****************************************************************************************/
    //! This procedure is called when user clicks save on exit-while-modified prompt
    function saveOnBackProcedure()
    {
        internal.exitAfterSave = true; //! Do exit after save

        if (checkValidity()) {
            if (!checkOverlappings()) {
                saveSchedule(); //! No prompt is needed
            }
        }
    }

    //! Checks schedule validity. Returns true if valid and false if invalid and shows error pop if
    //! needed
    function checkValidity()
    {
        if (!internal.scheduleToEdit) {
            return true;
        }

        var startTime = Date.fromLocaleTimeString(locale, internal.scheduleToEdit.startTime, "hh:mm AP");
        var endTime = Date.fromLocaleTimeString(locale, internal.scheduleToEdit.endTime, "hh:mm AP");

        const twoHoursToMs = 2 * 60 * 60 * 1000;

        if ((endTime - startTime) < 0) {
            endTime.setDate(endTime.getDate() + 1);
        }

        if ((endTime - startTime) < twoHoursToMs) {
            //! Show an error popup
            uiSession.popUps.errorPopup.errorMessage = "Schedule time period must be at least +2 hours.";
            uiSession.popupLayout.displayPopUp(uiSession.popUps.errorPopup, true);

            return false;
        }

        return true;
    }

    //! Checks overlap. Returns false if no overlappings. If there are overlappings prompt user for
    //! saving as enabled or disabled.
    function checkOverlappings()
    {
        // disabled, no need to check
        if (!schedule.enable)
            return false;

        //! Check overlapping schedules
        internal.overlappingSchedules = schedulesController.findOverlappingSchedules(
                    internal.scheduleToEdit.startTime, internal.scheduleToEdit.endTime,
                    internal.scheduleToEdit.repeats, schedule, // Exclude the original of this copy
                    schedule.active);

        //! New schedule overlaps with at least one other Schedule
        if (internal.overlappingSchedules.length > 0) {
            uiSession.popUps.scheduleOverlapPopup.accepted.connect(internal.saveEnabledSchedule);
            uiSession.popUps.scheduleOverlapPopup.rejected.connect(internal.saveDisabledSchedule);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.scheduleOverlapPopup);

            return true;
        } else {
            return false;
        }
    }

    //! This method saves editing data to _root.schedule. All of the save senarios will end up here
    function saveSchedule()
    {
        //! Apply internal.scheduleToEdit to _root.schedule and go back
        _root.schedule.name = internal.scheduleToEdit.name;
        _root.schedule.type = internal.scheduleToEdit.type;
        _root.schedule.minimumTemperature = internal.scheduleToEdit.minimumTemperature;
        _root.schedule.maximumTemperature = internal.scheduleToEdit.maximumTemperature;
        _root.schedule.humidity = internal.scheduleToEdit.humidity;
        _root.schedule.startTime = internal.scheduleToEdit.startTime;
        _root.schedule.endTime = internal.scheduleToEdit.endTime;
        _root.schedule.repeats = internal.scheduleToEdit.repeats;
        _root.schedule.dataSource = internal.scheduleToEdit.dataSource;
        _root.schedule.systemMode = internal.scheduleToEdit.systemMode;

        // Emit schedule changed to call updateCurrentSchedules function in schedule controller.
        appModel.schedulesChanged();

        //Displays a toast message for enabled schedule
        if (schedule.enable) {
            var dt = schedulesController.prepareToastMessage(schedule);
            uiSession.toastManager.showToast(dt.message, dt.detail);
        }

        // Edit schedule
        schedulesController.editScheduleInServer(schedule);
        deviceController.saveSettings();

        if (internal.exitAfterSave) {
            goBack();
        }
    }

    //! Pop this page from StackView
    function goBack()
    {
        if (_root.StackView.view) {
            //! Then Page is inside an StackView
            if (_root.StackView.view.currentItem == _root) {
                _root.StackView.view.pop();
            }
        }
    }
}

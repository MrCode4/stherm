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
    //! Device referenece
    property Device      device: uiSession?.appModel ?? null

    //! Schedule
    property ScheduleCPP schedule

    //! Whether temprature unit is Celsius
    property bool        isCelcius:  appModel.setting.tempratureUnit !== AppSpec.TempratureUnit.Fah

    //! Can schedule fields be editabled
    property bool        isEditable: false

    //!
    readonly property Schedule scheduleToDisplay: isEditable ? internal.scheduleToEdit : schedule

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
                confPop.accepted.connect(this, saveScheduleAndGoBack);
                confPop.rejected.connect(this, goBack)

                confPop.open();
                return;
            }
        }

        goBack();
    }

    /* Children
     * ****************************************************************************************/
    QtObject {
        id: internal

        //! A copy of _root.schedule in edit mode so user can preview changes and confirm before
        //! saving changes to the original schedule.
        property Schedule scheduleToEdit: Schedule { }

        function copySchedule()
        {
            if (_root.schedule) {
                scheduleToEdit.active = false; //! This is always false so overlapping check dont take
                                                //! place in ScheduleTimePage since this copy will
                                                //! overlap with itself
                scheduleToEdit.name = _root.schedule.name;
                scheduleToEdit.type = _root.schedule.type;
                scheduleToEdit.temprature = _root.schedule.temprature;
                scheduleToEdit.humidity = _root.schedule.humidity;
                scheduleToEdit.startTime = _root.schedule.startTime;
                scheduleToEdit.endTime = _root.schedule.endTime;
                scheduleToEdit.repeats = [..._root.schedule.repeats];
                scheduleToEdit.dataSource = _root.schedule.dataSource;
            }
        }

        function isScheduleModified()
        {
            return isEditable && _root.schedule && internal.scheduleToEdit
                    ? _root.schedule.name !== internal.scheduleToEdit.name
                      || _root.schedule.type !== internal.scheduleToEdit.type
                      || _root.schedule.temprature !== internal.scheduleToEdit.temprature
                      || _root.schedule.humidity !== internal.scheduleToEdit.humidity
                      || _root.schedule.startTime !== internal.scheduleToEdit.startTime
                      || _root.schedule.endTime !== internal.scheduleToEdit.endTime
                      || JSON.stringify(_root.schedule.repeats) !== JSON.stringify(internal.scheduleToEdit.repeats)
                      || _root.schedule.dataSource !== internal.scheduleToEdit.dataSource
                    : false
        }
    }

    //! Confirm change button
    ToolButton {
        id: confirmtBtn
        visible: internal.isScheduleModified()
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! Prompt user for save confirm
            //! It's might be better to show this only if this schedule is active since changing its
            //! properties may make it inactive
            confPop.message = "Save changes to schedule?"
            confPop.acceptText = qsTr("Yes");
            confPop.rejectText = qsTr("No");
            confPop.accepted.connect(this, saveScheduleAndGoBack);

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
                                                      "backButtonVisible": true,
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
                        text: scheduleToDisplay?.type ?? ""
                        elide: "ElideRight"
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTypePage.qml", {
                                                      "backButtonVisible": true,
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
                        text: "Temprature"
                    }

                    Label {
                        readonly property string unit: (device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                        ? "F" : "C") ?? "F"

                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: Number(Utils.convertedTemperature(scheduleToDisplay?.temprature ?? 0,
                                                                device.setting.tempratureUnit)
                                     ).toLocaleString(locale, "f", 0)
                              + ` \u00b0${unit}`
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTempraturePage.qml", {
                                                      "backButtonVisible": true,
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
                        text: "Humidity"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: scheduleToDisplay?.humidity ?? 0
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
                                                      "backButtonVisible": true,
                                                      "uiSession": uiSession,
                                                      "timeProperty": "start-time",
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
                                                      "backButtonVisible": true,
                                                      "uiSession": uiSession,
                                                      "timeProperty": "end-time",
                                                      "schedule": internal.scheduleToEdit,
                                                      "startTime": Date.fromLocaleTimeString(Qt.locale(),
                                                                                             scheduleToDisplay.startTime,
                                                                                             "hh:mm AP")
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
                            model: scheduleToDisplay?.repeats.split(",")
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
                                                      "backButtonVisible": true,
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
                        text: "Data Source"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: scheduleToDisplay?.dataSource ?? ""
                        elide: "ElideRight"
                    }
                }
            }
        }
    }

    ConfirmPopup {
        id: confPop
    }

    onIsEditableChanged: if (isEditable) internal.copySchedule()
    onScheduleChanged: if (isEditable) internal.copySchedule()

    function saveScheduleAndGoBack()
    {
        confPop.accepted.disconnect(saveScheduleAndGoBack);

        //! Apply internal.scheduleToEdit to _root.schedule and go back
        _root.schedule.name = internal.scheduleToEdit.name;
        _root.schedule.type = internal.scheduleToEdit.type;
        _root.schedule.temprature = internal.scheduleToEdit.temprature;
        _root.schedule.humidity = internal.scheduleToEdit.humidity;
        _root.schedule.startTime = internal.scheduleToEdit.startTime;
        _root.schedule.endTime = internal.scheduleToEdit.endTime;
        _root.schedule.repeats = [...internal.scheduleToEdit.repeats];
        _root.schedule.dataSource = internal.scheduleToEdit.dataSource;

        goBack();
    }

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

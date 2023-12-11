import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleDelegate provides a delegate for displaying schedules in ScheduleView
 * ***********************************************************************************************/
ItemDelegate {
    id: _root

    /* Signals
     * ****************************************************************************************/
    signal removed()

    /* Property declaration
     * ****************************************************************************************/
    //! UiSession
    property UiSession              uiSession

    //! SchedulesController
    property SchedulesController    schedulesController: uiSession?.schedulesController ?? null

    //! Schedule
    property Schedule               schedule

    //! Index in ListView
    property int                    delegateIndex

    /* Object properties
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    QtObject {
        id: internal

        property var overlappingSchedules: []
    }

    RowLayout {
        id: _delegateContent
        parent: _root.contentItem
        width: parent.width
        anchors.centerIn: parent
        spacing: 4

        //! Schedule icon
        RoniaTextIcon {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 24
            font.pointSize: Style.fontIconSize.smallPt
            text: {
                switch(schedule?.type) {
                case "Away":
                    return "\uf30d";
                case "Night":
                    return "\uf186"
                case "Home":
                    return "\uf015"
                case "Custom":
                    return "\uf1de"
                }

                return "-";
            }
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        //! Schedule name
        Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            font.bold: true
            text: schedule?.name ?? ""
            elide: "ElideRight"
        }

        //! Schedule repeat
        Item {
            Layout.preferredWidth: _fontMetric.advanceWidth("MuTuWeThFrSuSa") + 6
            Layout.preferredHeight: Material.delegateHeight
            opacity: 0.8

            RowLayout {
                anchors.centerIn: parent
                spacing: 1

                Repeater {
                    model: schedule?.repeats ?? 0
                    delegate: Label {
                        font: _fontMetric.font
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

        //! Enable switch
        Switch {
            id: _scheduleEnableSw
            checked: schedule?.active ?? false

            onToggled: {
                if (checked) {
                    //! First find if there is any overlapping schedules
                    if (uiSession) {
                        //! First check if this schedule has overlap with other Schedules
                        internal.overlappingSchedules = schedulesController.findOverlappingSchedules(
                                    Date.fromLocaleTimeString(Qt.locale(), schedule.startTime, "hh:mm AP"),
                                    Date.fromLocaleTimeString(Qt.locale(), schedule.endTime, "hh:mm AP"),
                                    schedule.repeats,
                                    schedule);

                        if (internal.overlappingSchedules.length > 0) {
                            //! First uncheck this Switch
                            toggle() //! This won't emit toggled() signal so no recursion occurs

                            uiSession.popUps.scheduleOverlapPopup.accepted.connect(setActive);
                            uiSession.popupLayout.displayPopUp(uiSession.popUps.scheduleOverlapPopup);
                            return;
                        }
                    }
                }

                if (schedule && schedule.active !== checked) {
                    schedule.active = checked;
                }
            }
        }

        //! Delete button
        ToolButton {
            contentItem: RoniaTextIcon {
                text: "\uf2ed"
            }

            onClicked: {
                //! Remove this item
                _removeAnima.running = true;
            }
        }
    }

    FontMetrics {
        id: _fontMetric
        font.pointSize: _root.font.pointSize * 0.85
    }

    ParallelAnimation {
        id: _removeAnima
        running: false
        loops: 1

        NumberAnimation {
            target: _root
            property: "opacity"
            to: 0
            duration: 200
        }

        NumberAnimation {
            target: _root
            property: "x"
            to: -_root.width
            duration: 200
        }

        onFinished: {
            removed();
        }
    }

    function setActive()
    {
        //! If there is overlapping Schedules disable them
        internal.overlappingSchedules.forEach((element, index) => {
                                                  element.active = false;
                                              });

        uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(setActive);

        if (schedule?.active === false) {
            schedule.active = true;
        }
    }
}


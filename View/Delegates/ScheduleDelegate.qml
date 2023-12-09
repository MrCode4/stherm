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
    //! Schedule
    property Schedule   schedule

    //! Index in ListView
    property int        delegateIndex

    /* Object properties
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    RowLayout {
        id: _delegateContent
        parent: _root.contentItem
        width: parent.width
        anchors.centerIn: parent
        spacing: 4

        //! Schedule icon
        RoniaTextIcon {
            Layout.alignment: Qt.AlignCenter
            Layout.rightMargin: 4
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
        }

        //! Schedule name
        Label {
            Layout.fillWidth: true
            font.bold: true
            text: schedule?.name ?? ""
        }

        //! Schedule repeat
        Item {
            Layout.preferredWidth: _fontMetric.boundingRect("MuTuWeThFrSuSa").width + 6 * 3
            Layout.preferredHeight: Material.delegateHeight
            opacity: 0.8

            RowLayout {
                anchors.centerIn: parent
                spacing: 3
                Repeater {
                    model: schedule?.repeats ?? 0
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

        //! Enable switch
        Switch {
            id: _scheduleEnableSw
            checked: schedule?.active ?? false

            onToggled: {
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
}


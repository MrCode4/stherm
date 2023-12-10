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
    //! Schedule
    property Schedule   schedule

    //! Whether temprature unit is Celsius
    property bool       isCelcius:  appModel.setting.tempratureUnit !== AppSpec.TempratureUnit.Fah

    //! Can schedule fields be editabled
    property bool       isEditable: true

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 8
    rightPadding: 4
    topPadding: 24
    title: "Schedule Preview"
    backButtonVisible: false
    titleHeadeingLevel: 3

    //! Back button is invisible in AddSchedulePage and only visible when editing an existing Schedule
    backButtonCallback: function() {
        if (pageStack.depth > 1) {
            pageStack.pop();
        } else if (_root.StackView.view) {
            _root.StackView.view.pop();
        }
    }

    /* Children
     * ****************************************************************************************/
    ToolButton {
        parent: isEditable ? _root.header.contentItem : _root
        visible: isEditable && pageStack.depth > 1
        enabled: Boolean(pageStack.currentItem?.isValid)
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            pageStack.pop();
        }
    }

    StackView {
        id: pageStack
        anchors.fill: parent

        initialItem: itemsFlickable
    }

    Flickable {
        id: itemsFlickable
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: _previewContentLay.implicitHeight
        contentWidth: width
        ScrollIndicator.vertical: ScrollIndicator { }

        ColumnLayout {
            id: _previewContentLay
            anchors {
                fill: parent
                rightMargin: 12
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
                        text: "Name"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: schedule?.name ?? ""
                        elide: "ElideRight"
                    }
                }

                onClicked: {
                    //! Open ScheduleNamePage for editing
                    pageStack.push("qrc:/Stherm/View/Schedule/ScheduleNamePage.qml", {
                                       "uiSession": uiSession,
                                       "schedule": _root.schedule
                                   });
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
                        text: schedule?.type ?? ""
                        elide: "ElideRight"
                    }
                }

                onClicked: {
                    //! Open ScheduleNamePage for editing
                    pageStack.push("qrc:/Stherm/View/Schedule/ScheduleTypePage.qml", {
                                       "uiSession": uiSession,
                                       "schedule": _root.schedule
                                   });
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
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: schedule?.temprature ?? 0
                    }
                }

                onClicked: {
                    //! Open ScheduleNamePage for editing
                    pageStack.push("qrc:/Stherm/View/Schedule/ScheduleTempraturePage.qml", {
                                       "uiSession": uiSession,
                                       "schedule": _root.schedule
                                   });
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
                        text: schedule?.humidity ?? 0
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
                        text: schedule?.startTime ?? ""
                    }
                }

                onClicked: {
                    //! Open ScheduleNamePage for editing
                    pageStack.push("qrc:/Stherm/View/Schedule/ScheduleTimePage.qml", {
                                       "uiSession": uiSession,
                                       "timeProperty": "start-time",
                                       "schedule": _root.schedule
                                   });
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
                        text: schedule?.endTime ?? ""
                    }
                }

                onClicked: {
                    //! Open ScheduleNamePage for editing
                    pageStack.push("qrc:/Stherm/View/Schedule/ScheduleTimePage.qml", {
                                       "uiSession": uiSession,
                                       "timeProperty": "end-time",
                                       "schedule": _root.schedule,
                                       "startTime": Date.fromLocaleTimeString(Qt.locale(),
                                                                              _root.schedule.startTime,
                                                                              "hh:mm AP")
                                   });
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
                            model: schedule?.repeats
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
                    //! Open ScheduleNamePage for editing
                    pageStack.push("qrc:/Stherm/View/Schedule/ScheduleRepeatPage.qml", {
                                       "uiSession": uiSession,
                                       "schedule": _root.schedule
                                   });
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
                        text: schedule?.dataSource ?? ""
                        elide: "ElideRight"
                    }
                }
            }
        }
    }
}

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
    property bool       isEditable: false

    /* Object properties
     * ****************************************************************************************/
    rightPadding: 4
    title: "Schedule Preview"
    backButtonVisible: false
    titleHeadeingLevel: 4

    /* Children
     * ****************************************************************************************/
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
                        text: schedule?.name ?? ""
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
                                                      "schedule": _root.schedule
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
                        text: schedule?.type ?? ""
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
                                                      "schedule": _root.schedule
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
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: schedule?.temprature ?? 0
                    }
                }

                onClicked: {
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTempraturePage.qml", {
                                                      "backButtonVisible": true,
                                                      "uiSession": uiSession,
                                                      "schedule": _root.schedule
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
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleTimePage.qml", {
                                                      "backButtonVisible": true,
                                                      "uiSession": uiSession,
                                                      "timeProperty": "start-time",
                                                      "schedule": _root.schedule
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
                        text: schedule?.endTime ?? ""
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
                                                      "schedule": _root.schedule,
                                                      "startTime": Date.fromLocaleTimeString(Qt.locale(),
                                                                                             _root.schedule.startTime,
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
                    if (!isEditable) return;
                    //! Open ScheduleNamePage for editing
                    if (_root.StackView.view) {
                        _root.StackView.view.push("qrc:/Stherm/View/Schedule/ScheduleRepeatPage.qml", {
                                                      "backButtonVisible": true,
                                                      "uiSession": uiSession,
                                                      "schedule": _root.schedule
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
                        text: schedule?.dataSource ?? ""
                        elide: "ElideRight"
                    }
                }
            }
        }
    }
}

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
    property Schedule   schedule
    property bool isCelcius : appModel.setting.tempratureUnit !== AppSpec.TempratureUnit.Fah

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 16
    rightPadding: 4
    topPadding: 32
    title: "Schedule Preview"
    backButtonVisible: false
    titleHeadeingLevel: 3

    /* Children
     * ****************************************************************************************/
    Flickable {
        anchors.fill: parent
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: _previewContentLay.implicitHeight
        contentWidth: width
        ScrollIndicator.vertical: ScrollIndicator { }

        RowLayout {
            id: _previewContentLay
            anchors {
                fill: parent
                rightMargin: 12
            }

            //! Schedule labels
            Pane {
                background: null
                padding: 0
                font.bold: true

                ColumnLayout {
                    Repeater {
                        model: [
                            "Name",
                            "Type",
                            "Temprature (\u00b0" + (isCelcius ? "C" : "F") + ")",
                            "Humidity",
                            "Start Time",
                            "End Time",
                            "Repeat",
                            "Data Source"
                        ]

                        delegate: Label {
                            Layout.fillWidth: true
                            Layout.preferredHeight: Material.delegateHeight
                            horizontalAlignment: "AlignJustify"
                            text: modelData
                        }
                    }
                }
            }

            //! Schedule values
            ColumnLayout {
                opacity: 0.7
                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Material.delegateHeight
                    horizontalAlignment: "AlignRight"
                    text: schedule?.name ?? ""
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Material.delegateHeight
                    horizontalAlignment: "AlignRight"
                    text: schedule?.type ?? ""
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Material.delegateHeight
                    horizontalAlignment: "AlignRight"
                    text: schedule?.temprature ?? 0
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Material.delegateHeight
                    horizontalAlignment: "AlignRight"
                    text: schedule?.humidity ?? 0
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Material.delegateHeight
                    horizontalAlignment: "AlignRight"
                    text: schedule?.startTime ?? ""
                }

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Material.delegateHeight
                    horizontalAlignment: "AlignRight"
                    text: schedule?.endTime ?? ""
                }

                //! Schedule
                RowLayout {
                    Layout.fillWidth: false
                    Layout.fillHeight: false
                    Layout.preferredHeight: Material.delegateHeight
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

                Label {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Material.delegateHeight
                    horizontalAlignment: "AlignRight"
                    text: schedule?.dataSource ?? ""
                }
            }
        }
    }
}

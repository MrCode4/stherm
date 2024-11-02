import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleSystemModeErrorPopup
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal duplicateSchedule()

    /* Object properties
     * ****************************************************************************************/
    title: "Error"
    titleBar: false

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        //! Header
        RowLayout {
            property int labelMargin: 0

            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillHeight: false

            spacing: 16

            //! Icon
            RoniaTextIcon {
                Layout.alignment: Qt.AlignCenter
                font.pointSize: Qt.application.font.pointSize * 2
                text: FAIcons.triangleExclamation
            }


            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 15
                horizontalAlignment: Text.AlignLeft
                textFormat: "MarkdownText"
                text: `${"#".repeat(titleHeadingLevel)} Error`
                elide: Text.ElideRight
                color: enabled ? Style.foreground : Style.hintTextColor
                linkColor: Style.linkColor
            }

            ToolButton {
                Layout.rightMargin: -root.rightPadding + 4
                contentItem: RoniaTextIcon {
                    font.pointSize: Application.font.pointSize * 1.2
                    text: FAIcons.xmark
                }

                onClicked: {
                    root.close();
                }

                Component.onCompleted: {
                    parent.labelMargin = Qt.binding(() => width);
                }
            }
        }

        Label {
            Layout.fillWidth: true

            text: `The schedule cannot be activated in the current ${AppSpec.systemModeToString(uiSession.appModel.systemSetup.systemMode)} system mode.\n` +
                  "To proceed, you can either create a new schedule that aligns with the current system mode or use the 'Duplicate' button to copy and adjust an existing schedule.";

            font.pointSize: Application.font.pointSize * 0.75
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 24
            spacing: 24

            ButtonInverted {
                Layout.preferredWidth: fontMetric.advanceWidth(" Duplicate ") + 6
                Layout.alignment: Qt.AlignLeft
                text: "OK"

                onClicked: {
                    close();
                }
            }

            ButtonInverted {
                Layout.preferredWidth: fontMetric.advanceWidth(" Duplicate ") + 6
                Layout.alignment: Qt.AlignRight
                text: "Duplicate"

                onClicked: {
                    duplicateSchedule()
                    close();
                }
            }
        }
    }

    FontMetrics {
        id: fontMetric
        font.pointSize: Application.font.pointSize
    }
}

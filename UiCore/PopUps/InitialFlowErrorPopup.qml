import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InitialFlowErrorPopup: Show initial flow errors.
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Error text
    property string errorMessage: ""

    property DeviceController deviceController

    property bool isBusy: false

    /* Signals
     * ****************************************************************************************/
    signal stopped();

    /* Object properties
     * ****************************************************************************************/
    width: AppStyle.size * 0.85
    height: AppStyle.size * 0.85
    titleBar: false

    onOpened: {
        if (NetworkInterface.hasInternet)
            deviceController.system.sendLog(false);
    }

    /* Children
     * ****************************************************************************************/

    ColumnLayout {

        anchors.fill: parent
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
                elide: "ElideRight"
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

        //! Serial number:
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: Style.button.buttonHeight

            Label {
                text: "Serial Number: "
                horizontalAlignment: Text.AlignLeft
                font.pointSize: Application.font.pointSize * 0.85
            }

            Label {
                text: deviceController.system.serialNumber
                horizontalAlignment: Text.AlignLeft
                font.pointSize: Application.font.pointSize * 0.7
            }
        }

        Flickable {
            id: errorFlick

            Layout.preferredHeight: root.height * 0.4
            Layout.fillWidth: true

            ScrollIndicator.vertical: ScrollIndicator {
                parent: errorFlick
                height: parent.height
                x: parent.width
            }

            clip: true
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: width
            contentHeight: errorLayout.implicitHeight

            Label {
                id: errorLayout

                anchors.fill: parent

                color: AppStyle.primaryRed
                text: errorMessage
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font.pointSize: Application.font.pointSize * 0.7
            }
        }

        ContactNuveSupportLabel {
            Layout.fillWidth: true
            Layout.preferredHeight: 35

            font.pointSize: Application.font.pointSize * 0.65
        }


        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: Style.button.buttonHeight
            visible: isBusy

            BusyIndicator {
                Layout.leftMargin: 10
                Layout.alignment: Qt.AlignLeft
                height: 45
                width: 45
                visible: isBusy
                running: isBusy
            }

            Label {
                Layout.fillWidth: true
                text: "Trying ..."
                horizontalAlignment: Text.AlignLeft
                font.pointSize: Application.font.pointSize * 0.9
            }

            //! Stop the current process button
            ButtonInverted {
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 10

                text: "Stop"
                onClicked: {
                    stopped();
                }
            }
        }
    }
}


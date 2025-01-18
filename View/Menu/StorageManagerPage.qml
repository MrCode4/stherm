import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm
import QtQuickStream

/*! ***********************************************************************************************
 * StorageManagerPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Manage Storage"

    /* Children
     * ****************************************************************************************/

    Flickable {
        id: _contentFlick

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        contentWidth: width
        contentHeight: _contentLay.implicitHeight + 4
        boundsMovement: Flickable.StopAtBounds

        ColumnLayout {
            id: _contentLay
            width: parent.width
            spacing: 8 * scaleFactor

            Label {
                text: qsTr("Drives:")
            }

            Label {
                text: qsTr("root:")
                font.pointSize: Application.font.pointSize * 0.8
            }

            Label {
                id: rootFreeBytes
                text: `Free bytes: ${AppUtilities.getStorageFreeBytes("/")}`
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    text =  `Available bytes: ${AppUtilities.getStorageFreeBytes("/")}`;
                }
            }

            Label {
                id: rootAvailableBytes
                text: qsTr(`Available bytes: ${AppUtilities.getStorageAvailableBytes("/")}`)
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    text =  qsTr(`Available bytes: ${AppUtilities.getStorageAvailableBytes("/")}`);
                }
            }

            RowLayout {
                Layout.preferredWidth: parent.width

                Label {
                    text: qsTr("/mnt/log:")

                    font.pointSize: Application.font.pointSize * 0.8
                }

                ButtonInverted {
                    Layout.alignment: Qt.AlignRight

                    text: qsTr("Clear")
                    font.pointSize: Application.font.pointSize * 0.8

                    onClicked: {
                        clearDirectoryConfirmPopup.directoryName = "/mnt/log";
                        clearDirectoryConfirmPopup.open();
                    }
                }
            }

            Label {
                id: mntFreeBytes

                text: qsTr(`Free bytes: ${AppUtilities.getStorageFreeBytes("/mnt/log")}`)
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    text =  qsTr(`Free bytes: ${AppUtilities.getStorageFreeBytes("/mnt/log")}`);
                }
            }

            Label {
                id: mntAvailableBytes

                text: qsTr(`Available bytes: ${AppUtilities.getStorageAvailableBytes("/mnt/log")}`)
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    text =  qsTr(`Available bytes: ${AppUtilities.getStorageAvailableBytes("/mnt/log")}`);
                }
            }

            Rectangle {
                Layout.preferredWidth: parent.width
                height: 3
                color: Style.foreground
            }

            Label {
                Layout.topMargin: 10

                text: qsTr("Directories:")
            }

            Repeater {
                Layout.preferredWidth: parent.width
                model: [...deviceController.system.usedDirectories(), "/usr/local/bin/backdoor"]

                delegate: GridLayout {
                    Layout.preferredWidth: parent.width
                    columns: 2

                    Label {
                        Layout.preferredWidth: parent.width * 0.7
                        Layout.alignment: Qt.AlignVCenter

                        text: modelData
                        font.pointSize: Application.font.pointSize * 0.8
                        wrapMode: Text.WordWrap
                    }

                    ButtonInverted {
                        Layout.alignment: Qt.AlignRight

                        text: qsTr("Clear")
                        font.pointSize: Application.font.pointSize * 0.8

                        onClicked: {
                            clearDirectoryConfirmPopup.directoryName = modelData;
                            clearDirectoryConfirmPopup.open();
                        }
                    }

                    Label {
                        id: modelDataSizeLabel

                        text: qsTr(`used: ${AppUtilities.getFolderUsedBytes(modelData)} bytes`)
                        font.pointSize: Application.font.pointSize * 0.8

                        function  update() {
                            text = qsTr(`used: ${AppUtilities.getFolderUsedBytes(modelData)} bytes`);
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: parent.width
                height: 3
                color: Style.foreground
            }

            Label {
                Layout.topMargin: 10

                text: qsTr("Files:")
            }

            Repeater {
                Layout.preferredWidth: parent.width

                model: [
                    "/test_results.csv",
                    "customIcon.png",
                    "/usr/local/bin/QSCore.cfg",
                    "/usr/local/bin/files_info.json",
                    "/usr/local/bin/updateInfoV1.json",
                    "/usr/local/bin/updateInfo.json",
                    "/usr/local/bin/sthermConfig.QQS.json",
                    "/usr/local/bin/output.bin",
                    "/usr/local/bin/override.ini"
                ]

                delegate: GridLayout {
                    columns: 2
                    Layout.preferredWidth: parent.width

                    Label {
                        Layout.preferredWidth: parent.width * 0.7
                        Layout.alignment: Qt.AlignVCenter

                        text: modelData
                        font.pointSize: Application.font.pointSize * 0.8
                        wrapMode: Text.WordWrap
                    }

                    ButtonInverted {
                        Layout.alignment: Qt.AlignRight

                        text: qsTr("Delete")
                        font.pointSize: Application.font.pointSize * 0.8

                        onClicked: {
                            deleteFileConfirmPopup.fileName = modelData;
                            deleteFileConfirmPopup.open();
                        }
                    }

                    Label {
                        id: modelDataFileSizeLabel


                        text: qsTr(`size: ${AppUtilities.getFileSizeBytes(modelData)} bytes`)
                        font.pointSize: Application.font.pointSize * 0.8

                        function  update() {
                            text = qsTr(`size: ${AppUtilities.getFileSizeBytes(modelData)} bytes`);
                        }
                    }
                }
            }
        }
    }

    Timer {
        id: updateTimer

        interval: 100
        repeat: false
        running: false

        onTriggered: {
            root.update();
        }
    }

    ConfirmPopup {
        id: clearDirectoryConfirmPopup

        property string directoryName: ""

        message: "Clear directory"
        detailMessage: `Are you sure you want to clear ${directoryName}?`

        onAccepted: {
            AppUtilities.removeDirectory(directoryName);

            modelDataSizeLabel.update();
            updateTimer.start();
        }
    }

    ConfirmPopup {
         id: deleteFileConfirmPopup

         property string fileName: ""

          message: "Delete file"
          detailMessage: `Are you sure you want to delete ${fileName}?`

          onAccepted: {
              QSFileIO.removeFile(fileName);

               modelDataFileSizeLabel.update();
               updateTimer.start();
          }
    }

    function update() {
        rootFreeBytes.update();
        rootAvailableBytes.update();
        mntFreeBytes.update();
        mntAvailableBytes.update();
    }
}

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
                font.pointSize: Application.font.pointSize * 0.85
            }

            Label {
                property int totalBytes: AppUtilities.getStorageTotalBytes("/")
                text: `  Total bytes: ${AppUtilities.bytesToNearestBigUnit(totalBytes)}`
                font.pointSize: Application.font.pointSize * 0.8
            }

            Label {
                id: rootFreeBytes


                font.pointSize: Application.font.pointSize * 0.8
                text: update();

                function update() {
                    let sizeBytres = AppUtilities.getStorageFreeBytes("/");
                    text = `  Free bytes: ${AppUtilities.bytesToNearestBigUnit(sizeBytres)}`

                    return text;
                }
            }

            Label {
                id: rootAvailableBytes

                text: update();
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    let sizeBytres = AppUtilities.getStorageAvailableBytes("/");
                    text =  qsTr(`  Available bytes: ${AppUtilities.bytesToNearestBigUnit(sizeBytres)}`);

                    return text;
                }
            }

            RowLayout {
                Layout.preferredWidth: parent.width

                Label {
                    text: qsTr("/mnt/log:")

                    font.pointSize: Application.font.pointSize * 0.85
                }

                ButtonInverted {
                    Layout.alignment: Qt.AlignRight

                    visible: false
                    text: qsTr("Clear")
                    font.pointSize: Application.font.pointSize * 0.8

                    onClicked: {
                        clearDirectoryConfirmPopup.directoryName = "/mnt/log";

                        clearDirectoryConfirmPopup.accepted.connect(this, update);
                        clearDirectoryConfirmPopup.hid.connect(this, disconect);

                        clearDirectoryConfirmPopup.open();
                    }

                    function update() {
                        AppUtilities.removeContentDirectory("/mnt/log");
                        updateTimer.start();
                    }

                    function disconect() {
                        clearDirectoryConfirmPopup.accepted.disconnect(this, update);
                        clearDirectoryConfirmPopup.hid.disconnect(this, disconect);
                    }
                }
            }

            Label {
                property int totalBytes: AppUtilities.getStorageTotalBytes("/mnt/log")
                text: `  Total bytes: ${AppUtilities.bytesToNearestBigUnit(totalBytes)}`

                font.pointSize: Application.font.pointSize * 0.8
            }

            Label {
                id: mntFreeBytes

                text: update()
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    let sizeBytes = AppUtilities.getStorageFreeBytes("/mnt/log");
                    text =  qsTr(`  Free bytes: ${AppUtilities.bytesToNearestBigUnit(sizeBytes)}`);

                    return text;
                }
            }

            Label {
                id: mntAvailableBytes

                text: update()
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    let sizeBytes = AppUtilities.getStorageAvailableBytes("/mnt/log");
                    text =  qsTr(`  Available bytes: ${AppUtilities.bytesToNearestBigUnit(sizeBytes)}`);

                    return text;
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

                property var dirs: [
                    ...deviceController.system.usedDirectories(),
                    "/usr/local/bin/backdoor"
                ]

                //! To detect the duplicate keys
                property var tempMap: []
                model: dirs.map(dir => {
                                    var dirNames = dir.split('/');
                                    var dirName = dirNames.pop();
                                    var key = dirName;

                                    // Handle duplicates within the map function
                                    while (tempMap.indexOf(key) != -1) {
                                        let tempKey = dirNames.pop();
                                        if ((tempKey?.length ?? 0) > 0) {
                                            key = `${tempKey}/${key}`;
                                        } else {
                                            break;
                                        }
                                    }

                                    tempMap.push(key);
                                    return {"key": key, "value": dir};
                                });


                delegate: GridLayout {
                    Layout.preferredWidth: parent.width
                    columns: 2
                    rowSpacing: -5

                    Label {
                        Layout.preferredWidth: parent.width * 0.7
                        Layout.alignment: Qt.AlignVCenter

                        text: modelData.key
                        font.pointSize: Application.font.pointSize * 0.8
                        wrapMode: Text.WordWrap
                    }

                    ButtonInverted {
                        Layout.alignment: Qt.AlignRight

                        text: qsTr("Clear")
                        font.pointSize: Application.font.pointSize * 0.8

                        onClicked: {
                            clearDirectoryConfirmPopup.directoryName = modelData.key;

                            clearDirectoryConfirmPopup.accepted.connect(this, update);
                            clearDirectoryConfirmPopup.hid.connect(this, disconnect);

                            clearDirectoryConfirmPopup.open();
                        }

                        function update() {
                            AppUtilities.removeContentDirectory(modelData.value);
                            modelDataSizeLabel.update();
                            updateTimer.start();
                        }

                        function disconnect() {
                            clearDirectoryConfirmPopup.accepted.disconnect(this, update);
                            clearDirectoryConfirmPopup.hid.disconnect(this, disconnect);
                        }
                    }

                    Label {
                        id: modelDataSizeLabel

                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignTop

                        text: update();
                        font.pointSize: Application.font.pointSize * 0.8

                        function  update() {
                            let size = AppUtilities.getFolderUsedBytes(modelData.value);
                            let sizeUnit = AppUtilities.bytesToNearestBigUnit(size);

                            text = qsTr(`used: ${sizeUnit}`);

                            return text;
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
                    { "key": "test_result",               "value": "/test_results.csv" },
                    { "key": "customIcon",                "value":"customIcon.png" } ,
                    { "key": "QSCore",                    "value":"/usr/local/bin/QSCore.cfg" },
                    { "key": "files_info",                "value":"/usr/local/bin/files_info.json" },
                    { "key": "updateInfoV1",              "value":"/usr/local/bin/updateInfoV1.json" },
                    { "key": "updateInfo",                "value":"/usr/local/bin/updateInfo.json" },
                    { "key": "sthermConfig",              "value":"/usr/local/bin/sthermConfig.QQS.json" },
                    { "key": "Proto output",              "value":"/usr/local/bin/output.bin" },
                    { "key": "override config",           "value":"/usr/local/bin/override.ini" }
                ]

                delegate: GridLayout {
                    Layout.preferredWidth: parent.width

                    columns: 2
                    rowSpacing: -5

                    Label {
                        Layout.preferredWidth: parent.width * 0.7
                        Layout.alignment: Qt.AlignVCenter

                        text: modelData.key
                        font.pointSize: Application.font.pointSize * 0.8
                        wrapMode: Text.WordWrap
                    }

                    ButtonInverted {
                        Layout.alignment: Qt.AlignRight

                        text: qsTr("Delete")
                        font.pointSize: Application.font.pointSize * 0.8

                        onClicked: {
                            deleteFileConfirmPopup.fileName = modelData.key;
                            deleteFileConfirmPopup.accepted.connect(this, update);
                            deleteFileConfirmPopup.hid.connect(this, disconnect);

                            deleteFileConfirmPopup.open();
                        }

                        function update() {
                            QSFileIO.removeFile(modelData.value);

                             modelDataFileSizeLabel.update();
                             updateTimer.start();
                        }

                        function disconnect() {
                            deleteFileConfirmPopup.accepted.disconnect(this, update);
                            deleteFileConfirmPopup.hid.disconnect(this, disconnect);
                        }
                    }

                    Label {
                        id: modelDataFileSizeLabel

                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignTop

                        text: update()
                        font.pointSize: Application.font.pointSize * 0.8

                        function  update() {
                            let size = AppUtilities.getFileSizeBytes(modelData.value);
                            let sizeUnit = AppUtilities.bytesToNearestBigUnit(size);

                            text = qsTr(`size: ${sizeUnit}`);

                            return text;
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
    }

    ConfirmPopup {
         id: deleteFileConfirmPopup

         property string fileName: ""

          message: "Delete file"
          detailMessage: `Are you sure you want to delete ${fileName}?`
    }

    function update() {
        rootFreeBytes.update();
        rootAvailableBytes.update();
        mntFreeBytes.update();
        mntAvailableBytes.update();
    }
}

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

    signal logPartitionCleared();

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
                text: qsTr("Partitions:")
            }

            Label {
                text: qsTr("Main:")
                font.pointSize: Application.font.pointSize * 0.85
            }

            Label {
                property int totalBytes: AppUtilities.getStorageTotalBytes("/")
                text: `  Total volume size: ${AppUtilities.bytesToNearestBigUnit(totalBytes)}`
                font.pointSize: Application.font.pointSize * 0.8
            }

            Label {
                id: rootFreeBytes


                font.pointSize: Application.font.pointSize * 0.8
                text: update();

                function update() {
                    let sizeBytres = AppUtilities.getStorageFreeBytes("/");
                    text = `  Free volume size: ${AppUtilities.bytesToNearestBigUnit(sizeBytres)}`

                    return text;
                }
            }

            Label {
                id: rootAvailableBytes

                text: update();
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    let sizeBytres = AppUtilities.getStorageAvailableBytes("/");
                    text =  qsTr(`  Available volume size: ${AppUtilities.bytesToNearestBigUnit(sizeBytres)}`);

                    return text;
                }
            }

            RowLayout {
                Layout.preferredWidth: parent.width

                Label {
                    text: qsTr("Log:")

                    font.pointSize: Application.font.pointSize * 0.85
                }

                ButtonInverted {
                    Layout.alignment: Qt.AlignRight

                    visible: true
                    text: qsTr("Clear")
                    font.pointSize: Application.font.pointSize * 0.8

                    onClicked: {
                        confirmPopup.message = "Clear partition"
                        confirmPopup.detailMessage = `Are you sure you want to clear Log partition content?`

                        confirmPopup.accepted.connect(this, removeAction);
                        confirmPopup.hid.connect(this, disconnectAction);

                        confirmPopup.open();
                    }

                    function removeAction() {
                        deviceController.system.removeLogPartition(false);
                        logPartitionCleared();
                        updateTimer.start();
                    }

                    function disconnectAction() {
                        confirmPopup.accepted.disconnect(this, removeAction);
                        confirmPopup.hid.disconnect(this, disconnectAction);
                    }
                }
            }

            Label {
                property int totalBytes: AppUtilities.getStorageTotalBytes("/mnt/log")
                text: `  Total volume size: ${AppUtilities.bytesToNearestBigUnit(totalBytes)}`

                font.pointSize: Application.font.pointSize * 0.8
            }

            Label {
                id: mntFreeBytes

                text: update()
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    let sizeBytes = AppUtilities.getStorageFreeBytes("/mnt/log");
                    text =  qsTr(`  Free volume size: ${AppUtilities.bytesToNearestBigUnit(sizeBytes)}`);

                    return text;
                }
            }

            Label {
                id: mntAvailableBytes

                text: update()
                font.pointSize: Application.font.pointSize * 0.8

                function update() {
                    let sizeBytes = AppUtilities.getStorageAvailableBytes("/mnt/log");
                    text =  qsTr(`  Available volume size: ${AppUtilities.bytesToNearestBigUnit(sizeBytes)}`);

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
                            confirmPopup.message = "Clear directory"
                            confirmPopup.detailMessage = `Are you sure you want to clear ${modelData.key}?`

                            confirmPopup.accepted.connect(this, update);
                            confirmPopup.hid.connect(this, disconnect);

                            confirmPopup.open();
                        }

                        function update() {
                            AppUtilities.removeContentDirectory(modelData.value);
                            modelDataSizeLabel.update();
                            updateTimer.start();
                        }

                        function disconnect() {
                            confirmPopup.accepted.disconnect(this, update);
                            confirmPopup.hid.disconnect(this, disconnect);
                        }
                    }

                    Label {
                        id: modelDataSizeLabel

                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignTop

                        text: update();
                        font.pointSize: Application.font.pointSize * 0.8

                        Connections {
                            target: root
                            function onLogPartitionCleared() {
                                modelDataSizeLabel.update();
                            }
                        }

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
                    { "key": "Test result",               "value": "/test_results.csv" },
                    { "key": "Vendor Icon",               "value": "/home/root/customIcon.png" } ,
                    { "key": "Core config",               "value": "/usr/local/bin/QSCore.cfg" },
                    { "key": "Updates data",              "value": "/usr/local/bin/files_info.json" },
                    { "key": "Updates info",              "value": "/usr/local/bin/updateInfoV1.json" },
                    { "key": "Obsolete files",            "value": "/usr/local/bin/updateInfo.json" },
                    { "key": "App config",                "value": "/usr/local/bin/sthermConfig.QQS.json" },
                    { "key": "Live data buffer",          "value": "/usr/local/bin/output.bin" },
                    { "key": "Sensors config",            "value": "/usr/local/bin/override.ini" }
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
                            confirmPopup.message = "Delete file"
                            confirmPopup.detailMessage = `Are you sure you want to delete the ${modelData.key} file?`

                            confirmPopup.accepted.connect(this, update);
                            confirmPopup.hid.connect(this, disconnect);

                            confirmPopup.open();
                        }

                        function update() {
                            QSFileIO.removeFile(modelData.value);

                             modelDataFileSizeLabel.update();
                             updateTimer.start();
                        }

                        function disconnect() {
                            confirmPopup.accepted.disconnect(this, update);
                            confirmPopup.hid.disconnect(this, disconnect);
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

    //! Confirm popup to clear and delete files.
    ConfirmPopup {
        id: confirmPopup
    }


    /* Functions
     * ****************************************************************************************/
    function update() {
        rootFreeBytes.update();
        rootAvailableBytes.update();
        mntFreeBytes.update();
        mntAvailableBytes.update();
    }
}

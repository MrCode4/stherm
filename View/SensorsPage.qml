import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorsPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    verticalPadding: 12
    title: "Sensors"
    footer: RowLayout {
        //! Add sensor button
        ToolButton {
            Layout.alignment: Qt.AlignRight
            horizontalPadding: 20
            contentItem: Row {
                spacing: parent.spacing

                RoniaTextIcon {
                    y: (parent.height - height) / 2
                    text: FAIcons.grid2Plus
                }

                Label {
                    y: (parent.height - height) / 2
                    text: "Add Sensor"
                }
            }

            onClicked: {
                //! Open AddSensorPage
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/Sensor/AddSensorPage.qml", {
                                                 "uiSession": root.uiSession
                                             });
                }
            }
        }
    }

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
        }
        width: Math.min(parent.width * 0.8, implicitWidth)
        height: Math.min(implicitHeight, parent.height)
        spacing: 16

        Button {
            Layout.alignment: Qt.AlignCenter
            text: "Onboard Sensor"
            checkable: true
            checked: true
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 16
            opacity: 0.8
            text: "Wireless Sensors"
        }

        //! Sensors ListView
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: _sensorsLv.implicitHeight

            ListView {
                id: _sensorsLv

                ScrollIndicator.vertical: ScrollIndicator {
                    x: parent.width + (root.contentItem.width - parent.width) / 2 - width - 4
                    parent: _sensorsLv.parent
                    height: parent.height - 30
                }

                anchors.fill: parent
                anchors.rightMargin: 10
                clip: true
                implicitHeight: contentHeight
                model: appModel?.sensors ?? []
                spacing: 12
                delegate: SensorDelegate {
                    required property var modelData
                    required property int index

                    width: ListView.view.width
                    height: implicitHeight
                    sensor: modelData instanceof Sensor ? modelData : null
                    delegateIndex: index

                    onClicked: {
                        //! Open SensorInfoPage for this sensor
                        if (root.StackView.view) {
                            root.StackView.view.push("qrc:/Stherm/View/Sensor/SensorInfoPage.qml", {
                                                         "sensor": sensor
                                                     });
                        }
                    }
                }
            }
        }
    }
}

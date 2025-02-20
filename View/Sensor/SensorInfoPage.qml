import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorInfoPage displays information for a given sensor and provides the ability to edit sensor
 * name or location
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Sensor to display its info
    required property Sensor sensor

    /* Object properties
     * ****************************************************************************************/
    bottomPadding: 24
    title: "Sensor Info"
    backButtonCallback: function() {
        if (mainStack.depth > 1) {
            mainStack.pop();
        } else {
            if (root.StackView.view) {
                //! Then Page is inside an StackView
                if (root.StackView.view.currentItem == root) {
                    root.StackView.view.pop();
                }
            }
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Confirma button which is only visible when sensor name or edit pages are in stack
    ToolButton {
        id: confirmBtn
        visible: mainStack.depth > 1
        parent: visible ? root.header.contentItem : root
        text: FAIcons.check

        onClicked: {
            if (mainStack.currentItem.apply instanceof Function) {
                mainStack.currentItem.apply();
                mainStack.pop();
            }
        }
    }

    StackView {
        id: mainStack

        anchors.fill: parent
        initialItem: infoLay
    }

    ColumnLayout {
        id: infoLay
        visible: false
        spacing: 4

        //! Name
        ItemDelegate {
            Layout.fillWidth: true

            horizontalPadding: 4
            background.implicitHeight: 56
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: "Name"
                }

                Label {
                    opacity: 0.8
                    text: sensor?.name ?? ""
                }
            }

            onClicked: {
                //! Open sensor name edit
                mainStack.push(sensorNamePageCompo);
            }
        }

        //! Location
        ItemDelegate {
            Layout.fillWidth: true

            horizontalPadding: 4
            background.implicitHeight: 56
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: "Location"
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    opacity: 0.8
                    text: AppSpec.sensorLocationNames[sensor?.location ?? 0]
                }
            }

            onClicked: {
                //! Open sensor location edit
                mainStack.push(sensorLocationPageCompo);
            }
        }

        //! Tempratur of sensor
        ItemDelegate {
            Layout.fillWidth: true

            horizontalPadding: 4
            background.implicitHeight: 56
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: "Temperature"
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    opacity: 0.8
                    text: "0"
                }
            }
        }

        //! Tempratur of sensor
        ItemDelegate {
            Layout.fillWidth: true

            horizontalPadding: 4
            background.implicitHeight: 56
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: "Humidity"
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    opacity: 0.8
                    text: "Humidifier"
                }
            }
        }

        //! Battery level
        ItemDelegate {
            Layout.fillWidth: true

            horizontalPadding: 4
            background.implicitHeight: 56
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: "Battery Level"
                }

                Label {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    opacity: 0.8
                    text: (sensor?.battery ?? 100) + " %"
                }
            }
        }

        //! Signal level
        ItemDelegate {
            Layout.fillWidth: true

            horizontalPadding: 4
            background.implicitHeight: 56
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    font.bold: true
                    text: "Signal Level"
                }

                Row {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.preferredHeight: 30
                    opacity: 0.8
                    spacing: 3

                    Repeater {
                        model: Math.ceil(81 / 20)
                        delegate: Rectangle {
                            y: parent.height - height
                            width: 2
                            height: 8 + index * 4
                            radius: 1
                        }
                    }
                }
            }
        }
    }

    //! Sensor name edit page
    Component {
        id: sensorNamePageCompo

        SensorNamePage {
            sensor: root.sensor
            sensorName: root.sensor?.name ?? ""

            Component.onCompleted: {
                confirmBtn.enabled = Qt.binding(() => sensorName.length > 0);
            }

            function apply()
            {
                if (sensor) {
                    sensor.name = sensorName;
                }
            }
        }
    }

    //! Sensor name edit page
    Component {
        id: sensorLocationPageCompo

        SensorLocationPage {
            sensor: root.sensor

            Component.onCompleted: {
                confirmBtn.enabled = Qt.binding(() => location !== AppSpec.SensorLocation.Unknown);
            }

            function apply()
            {
                if (sensor) {
                    sensor.location = location;
                }
            }
        }
    }
}

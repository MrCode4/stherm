import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleDataSourcePage provides a ui to select data source (sensor) for a schedule
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property Schedule   schedule

    //! Device reference
    property I_Device device: uiSession?.appModel ?? null

    //! Sensor that is selected
    readonly property Sensor sensor: buttonsGrp.checkedButton?.sensor ?? null

    /* Object properties
     * ****************************************************************************************/
    topPadding: 32
    title: "Data Source"
    backButtonVisible: false
    titleHeadeingLevel: 3

    /* Children
     * ****************************************************************************************/
    ButtonGroup {
        id: buttonsGrp
        buttons: sensorsBtnsLay.children
    }

    Flickable {
        id: sensorsFlick
        anchors.fill: parent
        clip: true
        contentHeight: sensorsBtnsLay.implicitHeight
        contentWidth: width
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: sensorsBtnsLay
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height
            width: implicitWidth

            SensorDelegate {
                Layout.fillWidth: true
                checkable: true
                checked: true
                sensor: Sensor {
                    name: "Onboard Sensor"
                }
            }

            Repeater {
                model: device?.sensors ?? 0
                delegate: SensorDelegate {
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    checkable: true
                    sensor: modelData instanceof Sensor ? modelData : null
                }
            }
        }
    }

    onSensorChanged: {
        if (schedule) {
            schedule.dataSource = sensor.name;
        }
    }
}

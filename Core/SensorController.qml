import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * SensorController handles sensors of the system
 * ***********************************************************************************************/
QSObject {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! List of device sensors
    property var        sensors: []

    /* Methods
     * ****************************************************************************************/
    function addSensorData(name, location)
    {
        var sensor = QSSerializer.createQSObject("Sensor", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
        sensor._qsRepo = AppCore.defaultRepo;
        sensor.name = name;
        sensor.location = location;

        sensors.push(sensor);
        sensorsChanged();
    }

    function addSensor(sensor: Sensor)
    {
        sensors.push(sensor);
        sensorsChanged();
    }

    function removeSensor(sensor: Sensor)
    {
        var sensorIndx = sensors.findIndex((element, index) => element === sensor);
        if (sensorIndx > -1) {
            var sensorToDelete = sensors.splice(sensorIndx, 1)[0];
            sensorToDelete.destroy();
        }
    }

    Component.onCompleted: {
        addSensorData("Hum Sensor", Sensor.Location.Unknown);
        addSensorData("CO2 Sens - Bedroom", Sensor.Location.Bedroom);
        addSensorData("Temp Sens - LR", Sensor.Location.LivingRoom);
        addSensorData("Temp Sens - Kitchen", Sensor.Location.Kitchen);
    }
}

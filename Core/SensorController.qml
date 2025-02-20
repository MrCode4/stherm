import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * SensorController handles sensors of the system
 * ***********************************************************************************************/
QtObject {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    property I_Device device

    Component.onCompleted: {
        device._sensors = []

        device.sensors.forEach(sensor => {
                                   // should check and try to connecting to all sensors
                                   console.log(sensor.name, sensor.location);
        });

        return; // uncomment this for testing!
        addSensorData("Hum Sensor", AppSpec.SensorLocation.Unknown);
        addSensorData("CO2 Sens - Bedroom", AppSpec.SensorLocation.Bedroom);
        addSensorData("Temp Sens - LR", AppSpec.SensorLocation.LivingRoom);
        addSensorData("Temp Sens - Kitchen", AppSpec.SensorLocation.Kitchen);
    }

    /* Methods
     * ****************************************************************************************/

    function addSensorData(name, location)
    {
        var sensor = QSSerializer.createQSObject("Sensor", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
        sensor._qsRepo = AppCore.defaultRepo;
        sensor.name = name;
        sensor.location = location;

        device._sensors.push(sensor);
        device._sensorsChanged();
    }

    function addSensor(sensor: Sensor)
    {
        device._sensors.push(sensor);
        device._sensorsChanged();
    }

    function removeSensor(sensor: Sensor)
    {
        var sensorIndx = device.sensors.findIndex((element, index) => element === sensor);
        if (sensorIndx > -1) {
            var sensorToDelete = device._sensors.splice(sensorIndx, 1)[0];
            sensorToDelete.destroy();
        }
    }


}

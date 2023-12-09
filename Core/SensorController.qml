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
        if (device.sensors.length !==0)
            return;

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

        device.sensors.push(sensor);
        device.sensorsChanged();
    }

    function addSensor(sensor: Sensor)
    {
        device.sensors.push(sensor);
        device.sensorsChanged();
    }

    function removeSensor(sensor: Sensor)
    {
        var sensorIndx = device.sensors.findIndex((element, index) => element === sensor);
        if (sensorIndx > -1) {
            var sensorToDelete = device.sensors.splice(sensorIndx, 1)[0];
            sensorToDelete.destroy();
        }
    }


}

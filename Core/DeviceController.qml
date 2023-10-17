import QtQuick

import Stherm

/*! ***********************************************************************************************
 * Device Controller
 *
 * ************************************************************************************************/
I_DeviceController {

    /* Property Declarations
     * ****************************************************************************************/

    /* Object Properties
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/

    /* Methods
     * ****************************************************************************************/
    function sendReceive(className, method, data)
    {
        var data_msg = {};

        var xhr = new XMLHttpRequest();
        xhr.open("POST", "/engine/index.php", false); //! Synchronous
        xhr.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
        xhr.send(JSON.stringify(data_msg));

        if (xhr.status === 200) {
            return JSON.parse(xhr.responseText);
        } else {
            console.error("Error in HTTP request:", xhr.status, xhr.statusText);
            return null;
        }
    }

    function updateBacklight()
    {
        console.log("starting rest for updateBacklight, color: ", device.backlight.color)
        //! Use a REST request to update device backlight
        var color = Qt.color(device.backlight.color);
        var r = color.r
        var g = color.g
        var b = color.b

        console.log("colors: ", r, ",", g, ",", b)
        var send_data = [color.r, color.g, color.b, 0, 'true']

        console.log("send data: ", send_data)
        sendReceive('hardware', 'setBacklight', send_data);
    }

    function updateFan()
    {
        console.log("starting rest for updateFan :", device.fan.working_per_hour)
        sendReceive('system', 'setFan', device.fan.working_per_hour);
    }
}

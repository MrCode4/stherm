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
        var data_msg = '{"request": {"class": "' + className + '", "method": "' + method + '", "params": ' + JSON.stringify(data) + '}}';

        let xhr = new XMLHttpRequest();

        xhr.onreadystatechange = function() {
            console.error("XMLHttpRequest onreadystatechange", xhr.readyState);

            if (xhr.readyState === XMLHttpRequest.DONE) {
                let response = {
                    status : xhr.status,
                    headers : xhr.getAllResponseHeaders(),
                    contentType : xhr.responseType,
                    content : xhr.response
                };
                console.error("XMLHttpRequest done", xhr.status, xhr.statusText, xhr.responseType);

                if (xhr.status === 200) {
                    console.error("XMLHttpRequest done", xhr.responseText, JSON.parse(xhr.responseText));
                } else {
                    console.error("Error in HTTP request:", xhr.status, xhr.statusText);
                }
            }
        }
        xhr.open("POST", "http://127.0.0.1/engine/index.php", true);
        xhr.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
        xhr.send(data_msg);
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

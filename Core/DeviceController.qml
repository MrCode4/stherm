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

    function updateDeviceBacklight()
    {
        console.log("starting rest for updateBacklight, color: ", device.backlight.color)

        //! Use a REST request to update device backlight
        var r = Math.round(device.backlight._color.r * 255)
        var g = Math.round(device.backlight._color.g * 255)
        var b = Math.round(device.backlight._color.b * 255)

        console.log("colors: ", r, ",", g, ",", b)
        //! RGB colors are also sent, maybe device preserve RGB color in off state too.
        var send_data = [r, g, b, 0, device.backlight.on ? "true" : "false"];

        console.log("send data: ", send_data)
        sendReceive('hardware', 'setBacklight', send_data);
    }

    function updateFan(mode: int, workingPerHour: int)
    {
        console.log("starting rest for updateFan :", workingPerHour)
        sendReceive('system', 'setFan', workingPerHour);

        // Updatew model
        device.fan.mode = mode
        device.fan.workingPerHour = workingPerHour
    }

    function setVacation(temp_min, temp_max, hum_min, hum_max)
    {
        if (!device)
            return;

        sendReceive('system', 'setVacation', [temp_min, temp_max, hum_min, hum_max]);

        device.vacation.temp_min = temp_min;
        device.vacation.temp_max = temp_max;
        device.vacation.hum_min  = hum_min;
        device.vacation.hum_max  = hum_max ;
    }

    function setSystemModeTo(systemMode: int)
    {
        if (systemMode >= 0 && systemMode <= AppSpec.SystemMode.Off) {
            //! Do required actions if any
            sendReceive('system', 'setMode', [ systemMode ]);

            device.systemMode = systemMode;
        }
    }

    //! Set device settings
    function setSettings(brightness, volume, temperatureUnit, timeFormat, reset, adaptive)
    {
        if (!device)
            return;

        console.log("Change settings to : ",
                    "brightness: ",     brightness,     "\n    ",
                    "volume: ",         volume,         "\n    ",
                    "temperature: ",    temperatureUnit,    "\n    ",
                    "timeFormat: ",     timeFormat,           "\n    ",
                    "reset: ",          reset,          "\n    ",
                    "adaptive: ",       adaptive,       "\n    "
                    );

        sendReceive('hardware', 'setSettings', [brightness, volume, temperatureUnit, timeFormat, reset, adaptive]);

        // Update setting when sendReceive is successful.
        if (device.setting.brightness !== brightness) {
            device.setting.brightness = brightness;
        }

        if (device.setting.volume !== volume) {
            device.setting.volume = volume;
        }

        if (device.setting.adaptiveBrightness !== adaptive) {
            device.setting.adaptiveBrightness = adaptive;
        }

        if (device.setting.timeFormat !== timeFormat) {
            device.setting.timeFormat = timeFormat;
        }

        if (device.setting.tempratureUnit !== temperatureUnit) {
            device.setting.tempratureUnit = temperatureUnit;
        }

    }

    //! Set temperature to device (system) and update model.
    function setDesiredTemperature(temperature: real) {
        sendReceive('system', 'setTemperature', [temperature]);

        // Update device temperature when setTemperature is successful.
        device.requestedTemp = temperature;
    }
}

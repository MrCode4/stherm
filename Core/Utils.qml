pragma Singleton

import QtQuick

import Stherm

/*! ***********************************************************************************************
 * Utils: Add general methods
 * ************************************************************************************************/

QtObject {

    //! Convert temperature from celsius to fahrenheit
    function convertedTemperature(celsiusTemp: real, toUnit: int) : real {
        return (toUnit === AppSpec.TempratureUnit.Fah) ? 32 + 1.8 * celsiusTemp : celsiusTemp;
    }

    //! Clamp temperature values based on min and max.
    //! For scheduled temperatures, use default min and max values.
    //! For others, set min and max values directly.
    function convertedTemperatureClamped(celsiusTemp: real, toUnit: int, min=null, max=null) : real {
        if (min === null || min === undefined)
            min = (toUnit === AppSpec.TempratureUnit.Fah ? AppSpec.minimumTemperatureF : AppSpec.minimumTemperatureC)

        if (max === null || max === undefined)
            max = (toUnit === AppSpec.TempratureUnit.Fah ? AppSpec.maximumTemperatureF : AppSpec.maximumTemperatureC)

        if (toUnit === AppSpec.TempratureUnit.Fah) {
            var fahTemp = 32 + 1.8 * celsiusTemp;
            return clampValue(fahTemp, min, max);
        } else {
            return clampValue(celsiusTemp, min, max);
        }
    }

    //! Clamp values
    function clampValue(value, min, max) {
        return Math.min(Math.max(value, min), max);
    }

    //! Convert temperature from fahrenheit to celsius
    function fahrenheitToCelsius(fahrTemp: real) : real {
        return (fahrTemp - 32) / 1.8;
    }
}

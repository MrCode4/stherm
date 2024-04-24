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

    function convertedTemperatureClamped(celsiusTemp: real, toUnit: int, min=null, max=null) : real {
        if (min === null || min === undefined)
            min = (toUnit === AppSpec.TempratureUnit.Fah ? AppSpec.minimumTemperatureF : AppSpec.minimumTemperatureC)

        if (max === null || max === undefined)
            max = (toUnit === AppSpec.TempratureUnit.Fah ? AppSpec.maximumTemperatureF : AppSpec.maximumTemperatureC)

        if (toUnit === AppSpec.TempratureUnit.Fah) {
            var fahTemp = 32 + 1.8 * celsiusTemp;
            return Math.min(Math.max(fahTemp, min), max);
        } else {
            return Math.min(Math.max(celsiusTemp, min), max);
        }
    }

    //! Convert temperature from fahrenheit to celsius
    function fahrenheitToCelsius(fahrTemp: real) : real {
        return (fahrTemp - 32) / 1.8;
    }
}

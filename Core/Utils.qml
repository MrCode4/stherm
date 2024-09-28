pragma Singleton

import QtQuick

import Stherm

/*! ***********************************************************************************************
 * Utils: Add general methods
 * ************************************************************************************************/

QtObject {

    //! Convert temperature from celsius to fahrenheit
    //! If no unit is specified (unit is null), AppSpec.defaultTemperatureUnit is used by default.
    function convertedTemperature(celsiusTemp: real, toUnit: int) : real {
        // Set null/undefined toUnit to default unit
        if (toUnit === null || toUnit === undefined) {
            toUnit = AppSpec.defaultTemperatureUnit;
        }

        return (toUnit === AppSpec.TempratureUnit.Cel) ? celsiusTemp : 32 + 1.8 * celsiusTemp;
    }

    //! Clamp temperature values based on min and max.
    //! For scheduled temperatures, use default min and max values.
    //! For others, set min and max values directly.
    //! If no unit is specified (unit is null), AppSpec.defaultTemperatureUnit is used by default.
    function convertedTemperatureClamped(celsiusTemp: real, toUnit: int, min=null, max=null) : real {
        // Set null/undefined toUnit to default unit
        if (toUnit === null || toUnit === undefined) {
            toUnit = AppSpec.defaultTemperatureUnit;
        }

        if (min === null || min === undefined)
            min = (toUnit === AppSpec.TempratureUnit.Cel ? AppSpec.minimumTemperatureC : AppSpec.minimumTemperatureF)

        if (max === null || max === undefined)
            max = (toUnit === AppSpec.TempratureUnit.Cel ? AppSpec.maximumTemperatureC : AppSpec.maximumTemperatureF)

        if (toUnit === AppSpec.TempratureUnit.Cel) {
            return clampValue(celsiusTemp, min, max);
        } else {
            var fahTemp = 32 + 1.8 * celsiusTemp;
            return clampValue(fahTemp, min, max);
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

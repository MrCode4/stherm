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

    //! Convert temperature from fahrenheit to celsius
    function fahrenheitToCelsius(fahrTemp: real) : real {
        return (fahrTemp - 32) / 1.8;
    }
}

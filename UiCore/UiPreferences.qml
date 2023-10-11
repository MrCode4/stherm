import QtQuick
import QtQuick.Controls
import QTemplateProject

QtObject {

    /* Property Declarations
     * ****************************************************************************************/
    // Display weights in the metric system or the imperial system
    property bool   isMetric: true
    property string touchMethod: "test"

    // Whether the window is fullscreen or windowed
    property string windowMode: "Hidden"

    /* Functions
     * ****************************************************************************************/
    function toGuiSOM(weight) : real
    {
        if (isMetric) {
            return weight
        }        
        console.log("To SOM: " + weight * 2.2046226218)
        return (weight * 2.2046226218)
    }

    function fromGuiSOM(weight) : real
    {
        if (isMetric) {
            return weight
        }
        console.log("From SOM: " + weight * 0.45359237)
        return (weight * 0.45359237)
    }

    function showWeightSuffix() : string
    {
        if (isMetric) {
            return "kg"
        }
        return "lbs"
    }
}

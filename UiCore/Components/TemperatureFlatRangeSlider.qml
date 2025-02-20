import QtQuick

import Ronia
import Ronia.impl

/*! ***********************************************************************************************
 * TemperatureFlatRangeSlider
 * ************************************************************************************************/
RangeSliderLabeled {
    id: control

    /* Property declaration
     * ****************************************************************************************/
    property color leftColor: "#ea0600"
    property color rightColor: "#0097cd"

    /* Object properties
     * ****************************************************************************************/
    showMinMax: true
    rightHandlerLaberOnTop: true

    background: Rectangle {
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        implicitWidth: control.horizontal ? 200 : 48
        implicitHeight: control.horizontal ? 48 : 200
        width: control.horizontal ? control.availableWidth : 4
        height: control.horizontal ? 4 : control.availableHeight
        scale: control.horizontal && control.mirrored ? -1 : 1

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 1.0
                color: enabled ? rightColor : Qt.darker(rightColor, _control.darkerShade)
            }

            GradientStop {
                position: 0.0
                color: enabled ? leftColor : Qt.darker(leftColor, _control.darkerShade)
            }
        }

        Rectangle {
            x: control.horizontal ? control.first.position * parent.width : 0
            y: control.horizontal ? 0 : control.second.visualPosition * parent.height
            z:1
            width: control.horizontal ? control.second.position * parent.width - control.first.position * parent.width : 4
            height: control.horizontal ? 4 : control.second.position * parent.height - control.first.position * parent.height

            color: control.enabled ? Style.accent : control.Material.sliderDisabledColor
        }
    }
}

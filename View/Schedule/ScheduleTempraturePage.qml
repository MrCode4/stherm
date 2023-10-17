import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 *  ScheduleTempraturePage is a page for setting temprature in AddSchedulePage
 * ***********************************************************************************************/
ScheduleBasePage {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    property int    temprature

    /* Object properties
     * ****************************************************************************************/
    title: "Temprature"

    /* Children
     * ****************************************************************************************/
    TickedSlider {
        readonly property int tickStepSize: 4

        anchors.centerIn: parent
        majorTickCount: ticksCount / 5
        ticksCount: 100 / tickStepSize
        from: 0
        to: 100
        stepSize: tickStepSize
    }
}

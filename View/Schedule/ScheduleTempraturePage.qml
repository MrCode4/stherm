import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 *  ScheduleTempraturePage is a page for setting temprature in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    property int    temprature

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                                        + implicitFooterHeight + topPadding + bottomPadding
                                                      : 0
    title: "Temprature"
    backButtonVisible: false

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

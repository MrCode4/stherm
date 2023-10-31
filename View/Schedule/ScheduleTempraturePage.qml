import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 *  ScheduleTempraturePage is a page for setting temprature in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Temprature value
    property alias      temprature: _tempSlider.value // conversion?

    property bool isCelcius :  appModel.setting.tempratureUnit !== AppSpec.TempratureUnit.Fah

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                                        + implicitFooterHeight + topPadding + bottomPadding
                                                      : 0
    topPadding: 24
    title: "Temprature (\u00b0" + (isCelcius ? "C" : "F") + ")"
    backButtonVisible: false
    titleHeadeingLevel: 3

    /* Children
     * ****************************************************************************************/
    TickedSlider {
        id: _tempSlider
        readonly property int tickStepSize: 4

        implicitWidth: implicitHeaderWidth * 3
        anchors.centerIn: parent
        from: isCelcius ? 18 : 65
        to: isCelcius ? 30 : 85
        majorTickCount: isCelcius ? 3 : 5
        ticksCount: to - from
        stepSize: 1

        ToolTip {
            parent: _tempSlider.handle
            y: -height - 16
            x: (parent.width - width) / 2
            visible: _tempSlider.pressed
            timeout: Number.MAX_VALUE
            delay: 0
            text: _tempSlider.value
        }
    }
}

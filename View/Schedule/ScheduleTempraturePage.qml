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
    //! Device reference
    property Device         device: uiSession?.appModel ?? null

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property ScheduleCPP    schedule

    //! Temprature is alwasy valid
    readonly property bool  isValid: true

    property bool editMode: false

    //!
    property bool           isCelcius:  appModel?.setting?.tempratureUnit === AppSpec.TempratureUnit.Cel

    //! Temprature value: this is always in celsius
    readonly property real  temprature: (isCelcius ? _tempSlider.value : Utils.fahrenheitToCelsius(_tempSlider.value))


    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                                        + implicitFooterHeight + topPadding + bottomPadding
                                                      : 0
    leftPadding: 8 * scaleFactor
    rightPadding: 8 * scaleFactor
    title: "Temprature (\u00b0" + (isCelcius ? "C" : "F") + ")"
    backButtonVisible: false
    titleHeadeingLevel: 4

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing
    ToolButton {
        parent: schedule ? _root.header.contentItem : _root
        visible: editMode
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            if (schedule && schedule.temprature !== temprature) {
                schedule.temprature = temprature;
            }

            backButtonCallback();
        }
    }

    TickedSlider {
        id: _tempSlider
        readonly property int tickStepSize: 4

        anchors.centerIn: parent
        width: parent.width
        from: isCelcius ? AppSpec.minimumTemperatureC : AppSpec.minimumTemperatureF
        to: isCelcius ? AppSpec.maximumTemperatureC : AppSpec.maximumTemperatureF
        value: Utils.convertedTemperatureClamped(schedule?.temprature ?? 0, device.setting.tempratureUnit)
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
            text: _tempSlider.value.toFixed(0)
        }
    }
}

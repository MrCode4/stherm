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

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property ScheduleCPP    schedule

    //! Temprature is alwasy valid
    readonly property bool  isValid: true

    property bool editMode: false

    property int temperatureUnit:      appModel?.setting?.tempratureUnit ?? AppSpec.defaultTemperatureUnit

    //! Is Celsius selected as the unit?
    readonly property bool           isCelcius:  temperatureUnit === AppSpec.TempratureUnit.Cel

    //! Temprature value: this is always in celsius
    readonly property real  temprature: (isCelcius ? _tempSlider.value : Utils.fahrenheitToCelsius(_tempSlider.value))

    //! Minimum temprature
    property real               minTemperature: deviceController?._minimumTemperatureUI ?? 40

    //! Maximum temprature
    property real               maxTemperature: deviceController?._maximumTemperatureUI ?? 90


    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                                        + implicitFooterHeight + topPadding + bottomPadding
                                                      : 0
    leftPadding: 8 * scaleFactor
    rightPadding: 8 * scaleFactor
    title: "Temprature (\u00b0" + (AppSpec.temperatureUnitString(temperatureUnit)) + ")"
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
            if (schedule) {
                if (temperatureUnit === AppSpec.TempratureUnit.Cel) {
                    schedule.minimumTemprature = _tempSlider.first.value;
                    schedule.maximumTemprature = _tempSlider.second.value;

                } else {
                    schedule.minimumTemprature = Utils.fahrenheitToCelsius(_tempSlider.first.value);
                    schedule.maximumTemprature = Utils.fahrenheitToCelsius(_tempSlider.second.value);
                }
            }

            backButtonCallback();
        }
    }

    TemperatureFlatRangeSlider {
        id: _tempSlider

        anchors.centerIn: parent
        width: parent.width

        from: minTemperature
        to: maxTemperature

        first.value: Utils.convertedTemperatureClamped(schedule?.minimumTemprature ?? from, temperatureUnit, minTemperature, maxTemperature)

        second.value: Utils.convertedTemperatureClamped(schedule?.maximumTemprature ?? to, temperatureUnit, minTemperature, maxTemperature)
        difference: temperatureUnit === AppSpec.TempratureUnit.Fah ? AppSpec.autoModeDiffrenceF : AppSpec.autoModeDiffrenceC


        labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(temperatureUnit))
        fromValueCeil: Utils.convertedTemperature(AppSpec.maxAutoMinTemp, temperatureUnit)
        toValueFloor: Utils.convertedTemperature(AppSpec.minAutoMaxTemp, temperatureUnit)
    }
}

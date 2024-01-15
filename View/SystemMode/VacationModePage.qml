import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * VacationModePage is a page used in SystemModePage to select vacation mode params
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Signals
     * ****************************************************************************************/
    signal saved();
    signal canceled();

    /* Property declaration
     * ****************************************************************************************/
    property Setting setting: appModel.setting

    //! System Accessories use in humidity control.
    property SystemAccessories systemAccessories: appModel.systemSetup.systemAccessories

    /* Object properties
     * ****************************************************************************************/
    title: "Vacation"
    backButtonCallback: function() {
        canceled();
        if (_root.StackView.view) {
            _root.StackView.view.pop();
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm icon
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            //! Apply settings and go back
            if (deviceController) {
                var minValue = _tempSlider.first.value;
                var maxValue = _tempSlider.second.value;

                if (setting.tempratureUnit === AppSpec.TempratureUnit.Fah) {
                    minValue = Utils.fahrenheitToCelsius(minValue)
                    maxValue = Utils.fahrenheitToCelsius(maxValue)
                }

                deviceController.setVacation(minValue, maxValue,
                                             _humSlider.first.value, _humSlider.second.value);
            }

            saved();
        }
    }

    GridLayout {
        anchors.centerIn: parent
        width: parent.width * 0.85

        columns: 2
        columnSpacing: 24
        rowSpacing: 12

        //! Temprature
        Label {
            Layout.columnSpan: 2
            text: "Temprature"
        }

        //! Temprature icon
        RoniaTextIcon {
            Layout.leftMargin: 24
            font.pointSize: _root.font.pointSize * 2
            text: "\uf2c8" //! temperature-three-quarters icon
        }

        //! Temprature range
        RangeSliderLabeled {
            id: _tempSlider
            Layout.fillWidth: true

            from: Utils.convertedTemperature(AppSpec.minimumTemperatureC, setting.tempratureUnit)
            to: Utils.convertedTemperature(AppSpec.maximumTemperatureC, setting.tempratureUnit)

            first.value: Utils.convertedTemperature(appModel?.vacation?.temp_min ?? from, setting.tempratureUnit)
            second.value: Utils.convertedTemperature(appModel?.vacation?.temp_max ?? to, setting.tempratureUnit)
            difference: setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? 10 : 6

            labelSuffix: "\u00b0" + (setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C")
        }

        //! Humidity
        Label {
            Layout.columnSpan: 2
            Layout.topMargin: height * 2
            text: "Humidity"

            visible: systemAccessories.accessoriesWireType !== AppSpecCPP.None
        }

        //! Humidity icon
        RoniaTextIcon {
            Layout.leftMargin: 24
            font.pointSize: _root.font.pointSize * 2
            text: "\uf750" //! droplet-percent icon
            visible: systemAccessories.accessoriesWireType !== AppSpecCPP.None
        }

        //! Humidity range
        RangeSliderLabeled {
            id: _humSlider
            Layout.fillWidth: true
            from: 0
            to: 100
            first.value: appModel?.vacation?.hum_min ?? from
            second.value: appModel?.vacation?.hum_max ?? to
            difference: 20
            labelSuffix: "%"
            visible: systemAccessories.accessoriesWireType !== AppSpecCPP.None
        }
    }
}

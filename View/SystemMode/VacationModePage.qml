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
                deviceController.setVacation(_tempSlider.first.value, _tempSlider.second.value,
                                             _humSlider.first.value, _humSlider.second.value);
            }

            saved();

            //! Go back
            if (_root.StackView.view) {
                _root.StackView.view.pop();
            }
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
            from: 0
            to: 60
            first.value: from
            second.value: to
            labelSuffix: "\u00b0C"
        }

        //! Humidity
        Label {
            Layout.columnSpan: 2
            Layout.topMargin: height * 2
            text: "Humidity"
        }

        //! Humidity icon
        RoniaTextIcon {
            Layout.leftMargin: 24
            font.pointSize: _root.font.pointSize * 2
            text: "\uf750" //! droplet-percent icon
        }

        //! Humidity range
        RangeSliderLabeled {
            id: _humSlider
            Layout.fillWidth: true
            from: 0
            to: 100
            first.value: from
            second.value: to
            labelSuffix: "%"
        }
    }
}

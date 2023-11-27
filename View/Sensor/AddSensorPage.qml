import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AddSensorPage provides the ability to add a new sensor
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Add Sensor"

    /* Children
     * ****************************************************************************************/
    StackView {
        id: _pageStack
        anchors.fill: parent

        initialItem: _sensorPairPage
    }

    SensorPairPage {
        id: _sensorPairPage
        visible: false

        //! For test: to add an arbitrary Sensor after two seconds
        Timer {
            interval: 2000
            running: true
            onTriggered: {
                _sensorPairPage.sensorPaired(Qt.createQmlObject(
                                                 `
                                                 import Stherm

                                                 Sensor { }
                                                 `, AppCore.defaultRepo));
            }
        }

        onSensorPaired: function(sensor) {
            if (sensor instanceof Sensor) {
                //! Push selecting sensor name and location pages
                _pageStack.push(sensorNamePageCompo, {
                                    "sensor": sensor
                                });
            }
        }
    }

    Component {
        id: sensorNamePageCompo

        SensorNamePage {
            id: namePage

            ToolButton {
                enabled: namePage.sensorName.length > 0
                parent: _root.header.contentItem
                contentItem: RoniaTextIcon {
                    text: FAIcons.arrowRight
                }

                onClicked: {
                    visible = false;
                    //! Set sensor name and go to selecting sensor loacation
                    namePage.sensor.name = namePage.sensorName;

                    _pageStack.push(sensorLocationPageCompo, {
                                        "sensor": sensor
                                    });
                }
            }
        }
    }

    Component {
        id: sensorLocationPageCompo

        SensorLocationPage {
            id: locationPage

            ToolButton {
                enabled: locationPage.location !== AppSpec.SensorLocation.Unknown
                visible: locationPage.visible
                parent: _root.header.contentItem
                contentItem: RoniaTextIcon {
                    text: FAIcons.check
                }

                onClicked: {
                    //! Save sensor location
                    locationPage.sensor.location = locationPage.location;
                    //! Set sensor repo and parent
                    locationPage.sensor._qsRepo = AppCore.defaultRepo;
                    uiSession.sensorController.addSensor(locationPage.sensor);

                    //! Pop AddSensorPage from its StackView
                    if (_root.StackView.view) {
                        _root.StackView.view.pop();
                    }
                }
            }
        }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AddSensorPage provides the ability to add a new sensor
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //!

    /* Object properties
     * ****************************************************************************************/
    title: pageStack.currentItem instanceof SensorLocationPage ? "Select Sensor Location" : "Add Sensor"
    backButtonCallback: function() {
        if (pageStack.depth > 1) {
            pageStack.pop();
        } else {
            if (root.StackView.view) {
                //! Then Page is inside an StackView
                if (root.StackView.view.currentItem == root) {
                    root.StackView.view.pop();
                }
            }
        }
    }

    Component.onCompleted: deviceController.updateEditMode(AppSpec.EMSensors);

    Component.onDestruction: deviceController.updateEditMode(AppSpec.EMSensors, false);

    /* Children
     * ****************************************************************************************/
    StackView {
        id: pageStack
        anchors.fill: parent

        initialItem: sensorPairPage
    }

    SensorPairPage {
        id: sensorPairPage

        property Sensor newSensor

        visible: false

        onSensorPaired: function(sensor) {
            if (sensor instanceof Sensor) {
                //! Push selecting sensor name and location pages
                pageStack.push(sensorNamePageCompo, {
                                    "sensor": sensor
                                });
            }
        }

        onSensorPairingCanceled : backButtonCallback()
    }

    Component {
        id: sensorNamePageCompo

        SensorNamePage {
            id: namePage

            ToolButton {
                id: backbutton
                visible: pageStack.depth === 2
                enabled: namePage.sensorName.length > 0
                parent: root.header.contentItem
                contentItem: RoniaTextIcon {
                    text: FAIcons.arrowRight
                }

                onClicked: {
                    //! Set sensor name and go to selecting sensor loacation
                    namePage.sensor.name = namePage.sensorName;

                    pageStack.push(sensorLocationPageCompo, {
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
                visible: pageStack.depth === 3
                parent: root.header.contentItem
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
                    sensorPairPage.newSensor = null;
                    if (root.StackView.view) {
                        root.StackView.view.pop();
                    }
                }
            }
        }
    }
}

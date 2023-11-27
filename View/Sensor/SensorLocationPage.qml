import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AddSensorLocationPage to select new sensor local
 * ***********************************************************************************************/
Page {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Sensor
    required property Sensor sensor

    //! Selected location for sensor
    readonly property int    location: locationsBtnGroup.checkedButton?.modelData.location ?? AppSpec.SensorLocation.Unknown

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: implicitWidth
    leftPadding: 8
    rightPadding: 20
    background: null

    /* Children
     * ****************************************************************************************/
    Flickable {
        id: locationsFlick

        property bool   showOthers: false
        property var    locations: [
            { "title": "Bedroom",       "location": AppSpec.SensorLocation.Bedroom,     "icon": "qrc:/Stherm/Images/Locations/bedroom.svg"      },
            { "title": "Living room",   "location": AppSpec.SensorLocation.LivingRoom,  "icon": "qrc:/Stherm/Images/Locations/living_room.svg"  },
            { "title": "Kids room",     "location": AppSpec.SensorLocation.KidsRoom,    "icon": "qrc:/Stherm/Images/Locations/kids_room.svg"    },
            { "title": "Bathroom",      "location": AppSpec.SensorLocation.Bathroom,    "icon": "qrc:/Stherm/Images/Locations/bathroom.svg"     },
            { "title": "Kitchen",       "location": AppSpec.SensorLocation.Kitchen,     "icon": "qrc:/Stherm/Images/Locations/kitchen.svg"      },
            { "title": "Basement",      "location": AppSpec.SensorLocation.Basement,    "icon": "qrc:/Stherm/Images/Locations/basement.svg"     },
            { "title": "Main floor",    "location": AppSpec.SensorLocation.MainFloor,   "icon": "qrc:/Stherm/Images/Locations/main_floor.svg"   },
            { "title": "Office",        "location": AppSpec.SensorLocation.Office,      "icon": "qrc:/Stherm/Images/Locations/office.svg"       },
            { "title": "Upstairs",      "location": AppSpec.SensorLocation.Upstairs,    "icon": "qrc:/Stherm/Images/Locations/upstairs.svg"     },
            { "title": "Downstairs",    "location": AppSpec.SensorLocation.Downstairs,  "icon": "qrc:/Stherm/Images/Locations/downstairs.svg"   },
            //! From here items should be shown as others
            { "title": "Dinning room",  "location": AppSpec.SensorLocation.DinningRoom, "icon": "qrc:/Stherm/Images/Locations/dining_room.svg"  },
            { "title": "Guesthouse",    "location": AppSpec.SensorLocation.Guesthouse,  "icon": "qrc:/Stherm/Images/Locations/guesthouse.svg"   },
            { "title": "Other",         "location": AppSpec.SensorLocation.Other,       "icon": "qrc:/Stherm/Images/Locations/other.svg"        },
        ]

        ScrollIndicator.vertical: ScrollIndicator {
            parent: _root
            x: parent.width - width - 2
            height: parent.height
        }

        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: locationsGrid.implicitHeight
        boundsBehavior: Flickable.StopAtBounds

        ButtonGroup {
            id: locationsBtnGroup
            buttons: locationsGrid.children
        }

        GridLayout {
            id: locationsGrid
            anchors.fill: parent
            columns: 3
            rowSpacing: 32
            columnSpacing: 16

            Repeater {
                model: locationsFlick.locations
                delegate: SensorLocationDelegate {
                    required property var modelData
                    required property int index

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    implicitWidth: 0 // ! So all items have same width
                    location: modelData
                    delegateIndex: index
                }
            }
        }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm
import "../Delegates"
/*! ***********************************************************************************************
 * AddSensorLocationPage to select new sensor local
 * ***********************************************************************************************/
Page {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Sensor
    required property Sensor sensor

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
            { "title": "Bedroom",           "icon": "qrc:/Stherm/Images/Locations/bedroom.svg" },
            { "title": "Living room",       "icon": "qrc:/Stherm/Images/Locations/living_room.svg" },
            { "title": "Kids room",         "icon": "qrc:/Stherm/Images/Locations/kids_room.svg" },
            { "title": "Bathroom",          "icon": "qrc:/Stherm/Images/Locations/bathroom.svg" },
            { "title": "Kitchen",           "icon": "qrc:/Stherm/Images/Locations/kitchen.svg" },
            { "title": "Basement",          "icon": "qrc:/Stherm/Images/Locations/basement.svg" },
            { "title": "Main floor",        "icon": "qrc:/Stherm/Images/Locations/main_floor.svg" },
            { "title": "Office",            "icon": "qrc:/Stherm/Images/Locations/office.svg" },
            { "title": "Upstairs",          "icon": "qrc:/Stherm/Images/Locations/upstairs.svg" },
            { "title": "Downstairs",        "icon": "qrc:/Stherm/Images/Locations/downstairs.svg" },
            //! From here items should be shown as others
            { "title": "Dinning room",      "icon": "qrc:/Stherm/Images/Locations/dining_room.svg" },
            { "title": "Guesthouse",        "icon": "qrc:/Stherm/Images/Locations/guesthouse.svg" },
            { "title": "Other",             "icon": "qrc:/Stherm/Images/Locations/other.svg" },
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

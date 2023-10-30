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
    implicitWidth: AppStyle.size
    implicitHeight: implicitWidth

    /* Object properties
     * ****************************************************************************************/
    background: null
    contentItem: GridView {
        property bool   showOthers: false
        property var    locations: [
                                       { "title": "Bedroom",           "icon": "" },
                                       { "title": "Living room",       "icon": "" },
                                       { "title": "Kids room",         "icon": "" },
                                       { "title": "Bathroom",          "icon": "" },
                                       { "title": "Kitchen",           "icon": "" },
                                       { "title": "Basement",          "icon": "" },
                                       { "title": "Main floor",        "icon": "" },
                                       { "title": "Office",            "icon": "" },
                                       { "title": "Upstairs",          "icon": "" },
                                       { "title": "Downstairs",        "icon": "" },
                                       //! From here items should be shown as others
                                       { "title": "Dinning room",      "icon": "" },
                                       { "title": "Guesthouse",        "icon": "" },
                                    ]

        cellWidth: width / 3
        model: {
            if (showOthers) {
                return locations;
            } else {
                var filteredLoc = locations.filter((elem, index) => index < locations.length - 2);
                filteredLoc.push({ "title": "Others", "icon": "" });
                return filteredLoc;
            }
        }
        delegate: ItemDelegate {
            text: modelData.title
        }
    }
}

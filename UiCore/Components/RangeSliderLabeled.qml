import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * A RangeSlider with labels for handles
 * ***********************************************************************************************/
RangeSlider {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Labels postfix
    property string     labelSuffix: ""

    /* Children
     * ****************************************************************************************/
    Label {
        anchors {
            top: parent.bottom
            horizontalCenter: parent.horizontalCenter
            topMargin: 6
        }
        parent: _root.first.handle
        font.pointSize: _root.font.pointSize * 0.9
        text: Number(_root.first.value).toLocaleString(locale, "f", 0) + labelSuffix
    }

    Label {
        anchors {
            top: parent.bottom
            horizontalCenter: parent.horizontalCenter
            topMargin: 6
        }
        parent: _root.second.handle
        font.pointSize: _root.font.pointSize * 0.9
        text: Number(_root.second.value).toLocaleString(locale, "f", 0) + labelSuffix
    }
}

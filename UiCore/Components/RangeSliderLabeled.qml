import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * A RangeSlider with labels for handles
 * ***********************************************************************************************/
LimitedRangeSlider {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Labels postfix
    property string     labelSuffix:    ""

    //! Show min and max values
    property bool       showMinMax:     false

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

    //! Min and Max value labels
    Loader {
        y: _root.background.y + _root.background.height + 4
        width: _root.width
        active: _root.showMinMax

        sourceComponent: Component {
            RowLayout {
                spacing: 0

                Label {
                    Layout.alignment: Qt.AlignLeading
                    Layout.leftMargin: -width / 2
                    opacity: 0.7
                    font.pointSize: _root.font.pointSize * 0.8
                    text: _root.from + labelSuffix
                }

                Label {
                    Layout.alignment: Qt.AlignTrailing
                    Layout.rightMargin: -width / 2
                    opacity: 0.7
                    font.pointSize: _root.font.pointSize * 0.8
                    text: _root.to + labelSuffix
                }
            }
        }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * RangeSliderLabeled A RangeSlider with labels for handles
 * ***********************************************************************************************/
LimitedRangeSlider {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Labels postfix
    property string     labelSuffix:    ""

    //! Left and right label prefix in the HTML format.
    property string     leftLabelPrefix:    ""
    property string     rightLabelPrefix:    ""

    //! Show min and max values
    property bool       showMinMax:     false

    //! Left handler label placement at the top
    property bool leftHandlerLaberOnTop: false

    //! Right handler label placement at the top
    property bool rightHandlerLaberOnTop: false

    /* Children
     * ****************************************************************************************/
    //! Left label
    Label {
        anchors {
            top: leftHandlerLaberOnTop ? undefined : parent.bottom
            bottom: leftHandlerLaberOnTop ? parent.top : undefined
            horizontalCenter: parent.horizontalCenter
            bottomMargin: 6
            topMargin: 6
        }
        parent: _root.first.handle
        font.pointSize: _root.font.pointSize * 0.9
        textFormat: Text.RichText
        lineHeight: 0.5
        text: leftHandlerLaberOnTop ? leftLabelPrefix + Number(_root.first.value).toLocaleString(locale, "f", 0) + labelSuffix:
                                      Number(_root.first.value).toLocaleString(locale, "f", 0) + labelSuffix + leftLabelPrefix
    }

    //! Right label
    Label {
        anchors {
            top: rightHandlerLaberOnTop ? undefined : parent.bottom
            bottom: rightHandlerLaberOnTop ? parent.top : undefined
            horizontalCenter: parent.horizontalCenter
            margins: 4
        }
        parent: _root.second.handle
        font.pointSize: _root.font.pointSize * 0.9
        textFormat: Text.RichText
        lineHeight: 0.5
        text: rightHandlerLaberOnTop ? rightLabelPrefix + Number(_root.second.value).toLocaleString(locale, "f", 0) + labelSuffix :
                                       Number(_root.second.value).toLocaleString(locale, "f", 0) + labelSuffix + rightLabelPrefix
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

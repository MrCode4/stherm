import QtQuick
import QtQuick.Controls

import Ronia
import Stherm

/*! ***********************************************************************************************
 * An Expandable Container
 * ************************************************************************************************/
Page {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    // property string title:          "Sample Title"

    property string subtitle:       ""

    property string subtitleColor:  "gray"

    //! Title Rectangle Color
    property string titleRectColor: ""

    property bool   isExpanded:      true

    property int    headerHeight:   35

    property int    maxHeight:      contentRect.implicitHeight + headerHeight

    property alias  subtitleText:   subtitleText

    default property alias contents: contentRect.data

    //! Bottom horizontal line under item visibility
    property bool bottomLineVisibility: true

    //! title rect opacity
    property real titleRectOpacity:     1.0

    //! Title of the expandable point size
    property int  titleFontPointSize: 12

    //! calculated height, based on inside item's height
    property real calculatedHeight:   maxHeight

    //! Whether item should be expandable or not
    property bool isExpandable:       true

    property Item toolButton:       Item {
        width: 25
        height: 25
    }

    onToolButtonChanged: {
        toolButton.parent = editorContainer;
        toolButton.anchors.right = editorContainer.right;
        toolButton.anchors.verticalCenter = editorContainer.verticalCenter;
        toolButton.anchors.rightMargin = 10;
    }

    /* Signal
     * ****************************************************************************************/
    signal clicked();

    /* Object Properties
     * ****************************************************************************************/
    // radius: 3
    // width: 200
    height: isExpanded ? Math.min(maxHeight, calculatedHeight) : headerHeight
    clip: true
    // color: "transparent"
    // border.color: control.activeFocus ? AppStyle.primaryColor : "transparent"
    // border.width: 1

    Behavior on height { NumberAnimation{ duration: 300; easing.type: Easing.InQuart} }

    /* Signals
     * ****************************************************************************************/

    /* Signal Handlers
     * ***************************************************************************************/

    /* Functions
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    //! Header
    Rectangle {
        width: parent.width
        anchors.top: parent.top
        height: headerHeight
        color: "transparent"

        //! background, for specifying color and opacity independent of the items inside
        Rectangle {
            anchors.fill: parent
            color: titleRectColor ? titleRectColor : "transparent"
            opacity: titleRectOpacity
        }

        // Expand/collapse icon
        RoniaTextIcon {
            id: expandableIcon
            text: isExpanded ? "\uf0d8" : "\uf0d7"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 10
            opacity: (isExpandable) ? 0.95 : 0.5
        }

        // Title
        Text {
            id: titleText
            text: control.title
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            color: AppStyle.primaryTextColor
            font.pointSize: titleFontPointSize
            opacity: 0.95
        }

        // Sub-Title
        Text {
            id: subtitleText
            text: control.subtitle
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: titleText.right
            anchors.leftMargin: 5
            color: subtitleColor
        }


        //! MouseArea
        TapHandler {
            onTapped: {
                control.isExpanded = !control.isExpanded;
                control.clicked();
            }
            enabled: isExpandable
        }

        //! Editor Container
        Item {
            id: editorContainer
            width: 100
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5
        }

    }

    //! Content Rectangle
    Item {
        id: contentRect
        anchors.fill: parent
        anchors.topMargin: control.headerHeight
    }

    // horizontal line
    Rectangle {
        width: parent.width
        anchors.bottom: parent.bottom
        height: 1
        color: "white"
        opacity: 0.2
        visible: bottomLineVisibility
    }
}

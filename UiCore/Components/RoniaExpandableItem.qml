import QtQuick
import QtQuick.Controls
import QTemplateProject

/*! ***********************************************************************************************
 * An Expandable Container
 * ************************************************************************************************/
Rectangle {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property string title:          "Sample Title"

    property string subtitle:       ""

    property string subtitleColor:  "gray"

    property string contentRectColor: AppStyle.backgroundGrayDarker1

    property bool   isExpanded:      true

    property int    headerHeight:   35

    property int    maxHeight:      contentRect.implicitHeight + headerHeight

    property alias  subtitleText:   subtitleText

    default property alias contents: contentRect.data

    property Item   toolButton:       Item {
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
    radius: 3
    width: 200
    height: isExpanded ? maxHeight: headerHeight
    clip: true
    color: AppStyle.backgroundGrayDarker1
    border.color: control.activeFocus ? AppStyle.primaryColor : "transparent"
    border.width: 1


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
    Item {
        width: parent.width
        anchors.top: parent.top
        height: headerHeight

        // Expand/collapse icon
        RoniaTextIcon {
            id: expandableIcon
            text: isExpanded ? "\uf0d8" : "\uf0d7"
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 10
            color: AppStyle.primaryTextColor
            opacity: 0.95
        }

        // Title
        Text {
            id: titleText
            text: control.title
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            color: AppStyle.primaryTextColor
            font.pointSize: 12
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
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                control.isExpanded = !control.isExpanded;
                control.clicked();
            }
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
//        color: contentRectColor
        anchors.fill: parent
        anchors.topMargin: control.headerHeight
        anchors.bottomMargin: control.radius
    }
}

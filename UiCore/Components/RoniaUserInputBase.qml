import QtQuick
import QtQuick.Controls
import QTemplateProject

Item {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property string name:           "Name"

    property string description:    name

    property Item   editor:         Item {
        width: 110
        height: 25
    }

    /* Object Properties
     * ****************************************************************************************/
    height: 35
    width: 200

    onEditorChanged: {
        editor.parent = editorContainer;
        editor.anchors.right = editorContainer.right;
        editor.anchors.verticalCenter = editorContainer.verticalCenter;
        editor.anchors.rightMargin = 10;
    }

    /* Children
     * ****************************************************************************************/
    //! Tooltip Text
    RoniaToolTip {
        text: description
        visible: infoIcon.infoMouseArea.containsMouse
        parent: infoIcon
        x: control.x
    }


    RoniaTextIcon {
        id: infoIcon
        text: "\uf05a"
        color: AppStyle.inputBoxGray
        font.pointSize: 12
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        property alias infoMouseArea: infoMouseArea

        MouseArea {
            id: infoMouseArea
            anchors.fill: parent
            anchors.margins: -5
            hoverEnabled: true
        }
    }

    //! Name
    Text {
        id: nameText
        text: control.name
        color: AppStyle.primaryTextColor
        opacity: 0.8
        font.pointSize: 10
        anchors.left: infoIcon.right
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter
    }

    //! Editor Container
    Item {
        id: editorContainer
        width: 110
        height: 25
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 10
    }
}

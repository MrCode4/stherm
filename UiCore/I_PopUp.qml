import QtQuick

import QTemplateProject

/*! ***********************************************************************************************
 * The I_PopUp is the interface/base class that should be extended by notification popups.
 * ************************************************************************************************/
Rectangle {
    id: popup

    /* Property Declarations
     * ****************************************************************************************/
    property bool   titleBar: false
    property string title: "title...."
    property string icon
    property alias  contentItem: contentItem
    property alias  titlebarRow: titlebarRow
    property alias  titleText: titleText
    property alias  backMouseArea: backMouseArea

    default property alias contents: contentItem.data

    /* Object Properties
     * ****************************************************************************************/
    // Invisible by default (shown on-demand)
    visible: false
    border.color: AppStyle.primaryColorDarker// "darkblue"
    radius: 15
    width: 467
    height: 427
    border.width: 2
    color: AppStyle.backgroundGray

    /* Signals
     * ****************************************************************************************/
    signal clicked()
    signal escPressed()


    /* Children
     * ****************************************************************************************/
    Row {
        id: titlebarRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 30
        anchors.topMargin: 15
        visible: titleBar
        spacing: 10

        RoniaTextIcon {
            text: icon
            font.weight: 800
            font.pixelSize: 20
            color: AppStyle.primaryColor
            anchors.verticalCenter: titleText.verticalCenter
        }

        Text {
            id: titleText
            text: title
            font.weight: 500
            font.pixelSize: 18
            font.family: "Roboto"
            color: AppStyle.primaryColor
        }

    }

    //! Mouse Area
    MouseArea {
        id: backMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: popup.clicked()
    }

    Keys.onEscapePressed: {
        uiSession.hidePopUp(popup);
    }

    Keys.onPressed: (event)=>{
        if (event.key === Qt.Key_Escape) {
            popup.escPressed()
            event.accepted = true;
        }
    }

    //! Content Item
    Item {
        id: contentItem
        anchors.fill: parent
        anchors.topMargin: 64
        anchors.leftMargin: 33
        anchors.bottomMargin: 16
        anchors.rightMargin: 32
    }

}

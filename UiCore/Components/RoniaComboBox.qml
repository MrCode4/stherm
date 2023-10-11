import QtQuick
import QtQuick.Controls
import QTemplateProject
import QtQuick.Layouts

/*! ***********************************************************************************************
 * Custome Combobox
 * ************************************************************************************************/
ComboBox {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property string iconText:       "\uf07b"

    property alias  backgroudRect:  backgroudRect

    property alias  indicatorRect:  indicatorRect

    property int    radius:         5

    property int    fontSize:       8

    property bool   isActive:       false

    /* Object Properties
     * ****************************************************************************************/
    model: ["First", "Second", "Third"]

    height: 35

    onActivated: {
        isActive = true
    }

    /* Children
     * ****************************************************************************************/
    delegate: ItemDelegate {
        width: control.width
        height: control.height
        contentItem: Text {
            text: modelData
            font.family: "Roboto"
            font.pointSize: fontSize
            font.weight: 400
            color: "white"
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: control.highlightedIndex === index
    }

    indicator: null


    contentItem: Row {
        spacing: 5

        Item {
            width: 10
            height: 10
        }

        RoniaTextIcon {
            text: iconText
            font.weight: 800
            font.pixelSize: 17
            visible: iconText.length > 0
            color: AppStyle.primaryColor
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            leftPadding: 0
            anchors.verticalCenter: parent.verticalCenter
            rightPadding: 0.15*control.width + control.spacing
            text: control.displayText
            font.family: "Roboto"
            font.pointSize: fontSize + 1
            font.weight: 400
            color: "white"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Rectangle {
        id: backgroudRect
        implicitWidth: 120
        implicitHeight: 35
        color: Qt.lighter(AppStyle.inputBoxGray, 1.1)// control.pressed ? AppStyle.inputBoxGray : AppStyle.inputBoxGray
        radius: control.radius

        Rectangle {
            id: indicatorRect
            color: AppStyle.inputBoxGray
            width: (0.15 * parent.width) > parent.height ? parent.height : 0.15 * parent.width
            height: parent.height - 2
            anchors.right: parent.right
            radius: parent.radius
            anchors.margins: 0
            anchors.verticalCenter: parent.verticalCenter

            RoniaTextIcon {
                text: "\uf0d7"
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: AppStyle.primaryTextColor
            }
        }
    }

    popup: Popup {
        y: control.height - 1
        width: control.width
        implicitHeight: contentItem.implicitHeight < 400 ? contentItem.implicitHeight : 400
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator { }
        }

        background: Rectangle {
            color: AppStyle.inputBoxGray
            radius: 2
        }
    }
}

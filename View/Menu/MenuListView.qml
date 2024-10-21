import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * MenuListView is a ListView to show menu items
 * ***********************************************************************************************/
ListView {
    id: root

    signal menuActivated(itemModel: var)

    ScrollIndicator.vertical: ScrollIndicator {
        id: scrollIndicator
    }
    implicitWidth: AppStyle.size
    implicitHeight: contentHeight
    clip: true

    delegate: ItemDelegate {
        id: delegate
        width: ListView.view.width
        height: (modelData?.visible ?? true) ? (modelData?.isSeparator ? 16 : implicitHeight) : 0
        visible: modelData?.visible ?? true
        leftPadding: 4 * scaleFactor
        contentItem: Loader {
            property int itemIndex: index
            property var itemData: modelData
            sourceComponent: itemData?.isSeparator ? compSeparator : compListItem
        }

        MouseArea {
            visible: !modelData.isSeparator
            anchors.fill: parent            
            propagateComposedEvents: true
            pressAndHoldInterval: 10000

            onClicked: {                    
                if (!modelData.prepareAndCheck ||
                        (modelData.prepareAndCheck instanceof Function && modelData.prepareAndCheck())) {
                    root.menuActivated(modelData);
                }
            }

            onPressAndHold: if (modelData.longPressAction instanceof Function) modelData.longPressAction();
        }
    }

    //! Use for gradient in the menu
    Rectangle {
        width: root.width
        height:  root.height * 0.45
        y:  ((scrollIndicator.position + scrollIndicator.size) < 0.97) ? root.height * 0.55 :  root.height


        gradient: Gradient {
            GradientStop { position: 0; color: Qt.alpha(AppStyle.backgroundColor, 0.0) }
            GradientStop { position: 1; color: Qt.alpha(AppStyle.backgroundColor, 1.0) }
        }

        //! Attach the animations
        //! Behaviour on y
        Behavior on y {
            enabled : ((scrollIndicator.position + scrollIndicator.size) > 0.97)
            NumberAnimation {
                duration: 1000
                easing.type: Easing.OutCubic
            }
        }
    }

    Component {
        id: compSeparator
        Item {
            Rectangle {
                anchors.centerIn: parent
                width: AppStyle.size
                height: 1
                Layout.preferredHeight: 1
                Layout.maximumHeight: 1
                color: Style.foreground
                opacity: 0.4
            }
        }
    }

    Component {
        id: compListItem
        RowLayout {
            spacing: 10 * scaleFactor
            //! Icon: this is supposed to be a unicode of Font Awesome
            RoniaTextIcon {
                id: fontAwesomeIcon
                visible: itemData?.icon ?? false
                Layout.preferredWidth: implicitHeight * 1.5
                text: itemData?.icon ?? ""
                color: enabled ? (itemData?.color ?? Style.foreground) : Style.hintTextColor
            }

            Image {
                id: imageIcon
                visible: !fontAwesomeIcon.visible
                fillMode: Image.PreserveAspectFit
                Layout.preferredWidth: implicitHeight
                source: visible ? (itemData?.image ?? "") : ""
            }

            //! Notification rectangle
            Rectangle {
                id: notificationRect
                parent: fontAwesomeIcon.visible ? fontAwesomeIcon : imageIcon
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: -10
                visible: itemData?.hasNotification ?? false
                width: 10
                height: 10
                radius: 5
                color: "red"
            }

            Label {
                Layout.fillWidth: true
                text: itemData?.text ?? ""
                verticalAlignment: "AlignVCenter"
                color: enabled ? (itemData?.color ?? Style.foreground) : Style.hintTextColor
            }
        }
    }
}

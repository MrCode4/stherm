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
    property var gotoView

    signal menuActivated(itemModel: var)

    ScrollIndicator.vertical: ScrollIndicator {}
    implicitWidth: AppStyle.size
    implicitHeight: contentHeight
    clip: true

    delegate: ItemDelegate {
        width: ListView.view.width
        height: (modelData?.visible ?? true) ? implicitHeight : 0
        visible: modelData?.visible ?? true
        leftPadding: 4 * scaleFactor
        contentItem: RowLayout {
            spacing: 10 * scaleFactor
            //! Icon: this is supposed to be a unicode of Font Awesome
            RoniaTextIcon {
                id: fontAwesomeIcon
                visible: modelData?.icon ?? false
                Layout.preferredWidth: implicitHeight * 1.5
                text: modelData?.icon ?? ""
            }

            Image {
                id: imageIcon
                visible: !fontAwesomeIcon.visible
                fillMode: Image.PreserveAspectFit
                Layout.preferredWidth: implicitHeight
                source: visible ? (modelData?.image ?? "") : ""
            }

            //! Notification rectangle
            Rectangle {
                id: notificationRect
                parent: fontAwesomeIcon.visible ? fontAwesomeIcon : imageIcon
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: -10
                visible: modelData?.hasNotification ?? false
                width: 10
                height: 10
                radius: 5
                color: "red"
            }

            Label {
                Layout.fillWidth: true
                text: modelData?.text ?? ""
                verticalAlignment: "AlignVCenter"
            }
        }

        MouseArea {
            anchors.fill: parent
            pressAndHoldInterval: 10000

            onClicked: {                    
                if (modelData.prepareAndCheck instanceof Function) {
                    if (modelData.prepareAndCheck() && modelData.view && root.gotoView instanceof Function) {
                        root.gotoView(modelData.view, modelData.properties);
                    }
                }
                else {
                    if (modelData.view && root.gotoView instanceof Function) {
                        root.gotoView(modelData.view, modelData.properties);
                    }
                }

                root.menuActivated(modelData);
            }

            onPressAndHold: if (modelData.longPressAction instanceof Function) modelData.longPressAction();
        }
    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * A delegate to be used in WifiPage
 * ***********************************************************************************************/
Control {
    id: _root

    /* Signals
     * ****************************************************************************************/
    signal clicked()

    /* Property declaration
     * ****************************************************************************************/
    //! WifiInfo
    property WifiInfo   wifi

    //! Index in ListView
    property int        delegateIndex

    /* Object properties
     * ****************************************************************************************/
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             _delegateContentRow.implicitHeight + topPadding + bottomPadding)
    hoverEnabled: true
    background: Rectangle {
        implicitHeight: Style.delegateHeight

        color: "transparent"

        Rectangle {
            anchors.fill: parent
            color: _root.hovered ? Style.rippleColor : "transparent"
        }
    }

    /* Children
     * ****************************************************************************************/
    ItemDelegate {
        id: _delegateButton
        width: parent.width
        height: Style.delegateHeight
        background: null

        RowLayout {
            id: _delegateContentRow
            x: 8
            width: parent.width - 16
            height: parent.height
            spacing: 12

            Item {
                Layout.preferredWidth: _wifiIcon.implicitWidth
                Layout.preferredHeight: _wifiIcon.implicitHeight

                RoniaTextIcon {
                    id: _wifiIcon
                    anchors.fill: parent
                    font.pointSize: _root.font.pointSize * 1.2
                    color: "gray"
                    opacity: 0.2
                    text: "\uf1eb"
                }
                RoniaTextIcon {
                    anchors.fill: parent
                    font.pointSize: _root.font.pointSize * 1.2
                    color: wifi?.connected ? Style.accent : Style.foreground
                    text: wifi ? ( wifi.strength > 80 ? "\uf1eb" //! wifi icon
                                                      : (wifi.strength > 50 ? "\uf6ab": //! wifi-fair icon
                                                                              (wifi.strength > 25 ?"\uf6aa" : "")//! wifi-weak icon
                                                         )) : ""
                }
            }

            ColumnLayout {
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    color: wifi?.connected ? Style.accent : Style.foreground
                    text: wifi?.ssid ?? ""
                    elide: "ElideRight"
                }

                Label {
                    opacity: 0.7
                    visible: wifi?.connected ?? false
                    font.pointSize: _root.font.pointSize * 0.8
                    color: wifi?.connected ? Style.accent : Style.foreground
                    text: "Connected"
                }
            }
        }

        onClicked: _root.clicked();
    }

    Behavior on height { NumberAnimation { } }
}


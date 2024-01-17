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
            width: parent.width - 12
            height: parent.height
            spacing: 12

            Item {
                Layout.fillHeight: true
                Layout.topMargin: 12 * scaleFactor
                Layout.bottomMargin: 12 * scaleFactor
                Layout.preferredWidth: height

                WifiIcon {
                    anchors.fill: parent

                    isConnected: true //! Disconnect icon is not desired
                    strength: wifi?.strength ?? 0
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

            BusyIndicator {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.topMargin: 8
                Layout.bottomMargin: 8

                implicitWidth: 0
                implicitHeight: 0
                visible: wifi?.isConnecting ?? false
                running: visible
            }
        }

        onClicked: _root.clicked();
    }

    Behavior on height { NumberAnimation { } }
}


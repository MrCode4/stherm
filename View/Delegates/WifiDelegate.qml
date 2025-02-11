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
    signal forgetClicked()
    signal disconnectClicked()
    signal connectClicked()


    /* Property declaration
     * ****************************************************************************************/
    //! WifiInfo
    property WifiInfo wifi

    //! Index in ListView
    property int delegateIndex

    property bool isWPA3: false

    property bool wifiInRange: true

    property bool isSelected: false

    /* Object properties
     * ****************************************************************************************/
    implicitHeight: Math.max(
                        implicitBackgroundHeight + topInset + bottomInset,
                        _delegateContentRow.implicitHeight + topPadding + bottomPadding)
    hoverEnabled: false
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

                    isConnected: wifi?.connected ?? false
                    hasInternet: NetworkInterface.hasInternet
                    strength: wifi?.strength ?? 0

                    anchors {
                        fill: parent
                    }
                }

                RoniaTextIcon {
                    id: shieldTextIcon

                    visible: _root.isWPA3 && lockTextIcon.visible
                    text: FAIcons.shield

                    font {
                        pointSize: Application.font.pointSize * 0.9
                    }

                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                        rightMargin: -4
                        bottomMargin: -4
                    }
                }

                // security indicator
                RoniaTextIcon {
                    id: lockTextIcon

                    //! strength > 0 means don't display lock for non-in-range wifis
                    visible: wifi?.strength > 0 && wifi?.security !== ""
                    text: FAIcons.lock
                    color: enabled ? _root.isWPA3 ? Style.background : Style.foreground : Style.hintTextColor

                    font {
                        pointSize: Application.font.pointSize * 0.5
                    }

                    anchors {
                        centerIn: _root.isWPA3 ? shieldTextIcon : undefined
                        right: _root.isWPA3 ? undefined : parent.right
                        bottom: _root.isWPA3 ? undefined : parent.bottom
                    }
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
                    color: wifi?.connected ? Style.accent : Style.foreground
                    text: "Connected"

                    font {
                        pointSize: _root.font.pointSize * 0.8
                    }
                }
            }

            ItemDelegate {
                Layout.alignment: Qt.AlignRight | Qt.AlignHCenter
                Layout.fillHeight: true

                visible: wifi && wifiInRange && wifi.connected && !wifi.isConnecting
                hoverEnabled: false

                rightPadding: 4
                leftPadding: 8

                contentItem: Label {
                    anchors.centerIn: parent

                    color: Style.foreground
                    font.pointSize: Qt.application.font.pointSize * 0.9
                    font.bold: true
                    text: " Disconnect "
                    verticalAlignment: Text.AlignVCenter
                }

                background: null

                onClicked: {
                    if (!isSelected)
                        _root.clicked()

                    disconnectClicked();
                }
            }

            ItemDelegate {
                Layout.alignment: Qt.AlignRight | Qt.AlignHCenter
                Layout.fillHeight: true

                visible: wifi && wifiInRange && !wifi?.connected && !wifi.isConnecting && _root.isSelected
                hoverEnabled: false

                rightPadding: 4
                leftPadding: 8

                contentItem: Label {
                    anchors.centerIn: parent

                    color: Style.foreground
                    font.pointSize: Qt.application.font.pointSize * 0.9
                    font.bold: true
                    text: " Connect "
                    verticalAlignment: Text.AlignVCenter
                }

                background: null

                onClicked: {
                    connectClicked();
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

            ToolButton {
                id: forgetBtn

                Layout.alignment: Qt.AlignCenter
                visible: (wifi?.isSaved && !wifi?.connected) ?? false

                contentItem: RoniaTextIcon {
                    font.pointSize: Style.fontIconSize.normalPt
                    color: Style.foreground
                    text: FAIcons.xmark
                }

                onClicked: {
                    forgetClicked()
                }
            }
        }

        onClicked: {
            _root.clicked()
        }
    }

    Behavior on height {
        NumberAnimation {}
    }
}

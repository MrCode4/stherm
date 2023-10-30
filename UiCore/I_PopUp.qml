import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import Ronia
import Stherm

/*! ***********************************************************************************************
 * The I_PopUp is the interface/base class that should be extended by notification popups.
 * ************************************************************************************************/
Popup {
    id: _popup

    /* Signals
     * ****************************************************************************************/
    signal clicked()
    signal escPressed()

    /* Property Declarations
     * ****************************************************************************************/
    //! Whether header bar is visible or not
    property bool       titleBar:           true

    //! Title of popup
    property string     title:              "Popup"

    //! H level of Popup title
    property int        titleHeadingLevel:  3

    //! Icon of popup: this should be a font-awesome icon
    property string     icon:               ""

    //! Content pane
    property alias      contentControl:     _contentControl

    //! Popup content item
    default property alias contents:        _contentControl.data

    /* Object Properties
     * ****************************************************************************************/
    anchors.centerIn: T.Overlay.overlay ?? parent
    implicitWidth: Math.min(_mainCol.implicitWidth + leftPadding + rightPadding,
                            (T.Overlay.overlay?.width ?? AppStyle.size) * 0.80)
    implicitHeight: Math.min(_mainCol.implicitHeight + bottomPadding + topPadding,
                             (T.Overlay.overlay?.height ?? AppStyle.size) * 0.80)
    spacing: 24
    horizontalPadding: 16
    topPadding: 4
    bottomPadding: 24
    dim: true
    modal: true
    closePolicy: Popup.CloseOnReleaseOutside | Popup.CloseOnEscape

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: _mainCol
        anchors.fill: parent
        spacing: _popup.spacing

        //! Header
        RowLayout {
            id: _header
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillHeight: false

            visible: titleBar
            spacing: 16

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: _closeBtn.width
                horizontalAlignment: "AlignHCenter"
                textFormat: "MarkdownText"
                text: `${"#".repeat(titleHeadingLevel)} ${title}`
                elide: "ElideRight"
            }

            ToolButton {
                id: _closeBtn
                Layout.rightMargin: -_popup.rightPadding + 4
                highlighted: true
                contentItem: RoniaTextIcon {
                    font.pointSize: Application.font.pointSize * 1.2
                    text: FAIcons.xmark
                }

                onClicked: {
                    _popup.close();
                }
            }
        }

        //! Icon
        RoniaTextIcon {
            Layout.alignment: Qt.AlignCenter
            visible: Boolean(icon)
            font.pointSize: Qt.application.font.pointSize * 2.4
            text: icon
        }

        //! Content
        Control {
            id: _contentControl
            Layout.fillHeight: true
            Layout.fillWidth: true
            implicitWidth: children.length > 0 ? children[0].implicitWidth ?? (AppStyle * 0.7) : AppStyle * 0.7
            implicitHeight: children.length > 0 ? children[0].implicitHeight ?? (AppStyle * 0.7) : AppStyle * 0.7
            background: null
        }
    }
}

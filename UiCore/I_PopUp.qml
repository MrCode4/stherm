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
    //! This signal is emitted when this I_PopUp is about to hide, carrying a ref to this instance
    signal hid(I_PopUp popup)

    //! This signal is emitted when this I_PopUp is destructed, carrying a ref to this instance
    signal destructed(I_PopUp popup)

    /* Property Declarations
     * ****************************************************************************************/
    //! Whether header bar is visible or not
    property bool                       titleBar:           true

    //! Title of popup
    property string                     title:              "Popup"

    //! H level of Popup title
    property int                        titleHeadingLevel:  3

    //! Icon of popup: this should be a font-awesome icon
    property string                     icon:               ""

    //! Popup content item
    default property list<QtObject>     contents: []

    /* Object Properties
     * ****************************************************************************************/
    anchors.centerIn: T.Overlay.overlay ?? parent
    implicitWidth: Math.min(implicitContentWidth + leftPadding + rightPadding,
                            (T.Overlay.overlay?.width ?? AppStyle.size) * 0.85)
    implicitHeight: Math.min(implicitContentHeight + bottomPadding + topPadding,
                             (T.Overlay.overlay?.height ?? AppStyle.size) * 0.85)
    spacing: 24
    horizontalPadding: 16
    topPadding: 4
    bottomPadding: 24
    dim: true
    modal: true
    closePolicy: Popup.CloseOnReleaseOutside | Popup.CloseOnEscape

    onAboutToHide: hid(this)
    Component.onDestruction: destructed(this)

    /* Children
     * ****************************************************************************************/
    //! There should be no id in contentItem's children so deferred binding execution is properly done
    contentItem: ColumnLayout {
        spacing: _popup.spacing

        //! Header
        RowLayout {
            property int labelMargin: 0

            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            Layout.fillHeight: false

            visible: titleBar
            spacing: 16

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: parent.labelMargin
                horizontalAlignment: "AlignHCenter"
                textFormat: "MarkdownText"
                text: `${"#".repeat(titleHeadingLevel)} ${title}`
                elide: "ElideRight"
                color: enabled ? Style.foreground : Style.hintTextColor
                linkColor: Style.linkColor
            }

            ToolButton {
                Layout.rightMargin: -_popup.rightPadding + 4
                highlighted: true
                contentItem: RoniaTextIcon {
                    font.pointSize: Application.font.pointSize * 1.2
                    text: FAIcons.xmark
                }

                onClicked: {
                    _popup.close();
                }

                Component.onCompleted: {
                    parent.labelMargin = Qt.binding(() => width);
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
            Layout.fillHeight: true
            Layout.fillWidth: true
            implicitWidth: children.length > 0 ? children[0].implicitWidth ?? (AppStyle * 0.7) : AppStyle * 0.7
            implicitHeight: children.length > 0 ? children[0].implicitHeight ?? (AppStyle * 0.7) : AppStyle * 0.7
            data: contents
            background: null
        }
    }
}

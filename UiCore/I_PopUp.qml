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

    //! Default property
    default property alias contents:        _contentPane.data

    /* Object Properties
     * ****************************************************************************************/
    verticalPadding: 6
    horizontalPadding: 12
    spacing: 8
    leftMargin: T.Overlay.overlay.width * 0.08
    rightMargin: T.Overlay.overlay.width * 0.08
    topMargin: T.Overlay.overlay.height * 0.08
    bottomMargin: T.Overlay.overlay.height * 0.08
    closePolicy: Popup.CloseOnReleaseOutside | Popup.CloseOnEscape

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: _mainCol
        anchors.fill: parent
        spacing: 8

        //! Header
        RowLayout {
            id: _header
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

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

            RoniaTextIcon {
                id: _closeBtn
                font.pointSize: Qt.application.font.pointSize * 1.5
                text: "\uf00d" //! xmark icon

                TapHandler {
                    onTapped: {
                        _popup.close();
                    }
                }
            }
        }

        //! Icon
        RoniaTextIcon {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: _popup.spacing
            visible: Boolean(icon)
            font.pointSize: Qt.application.font.pointSize * 1.5
            text: icon
        }

        //! Content
        Pane {
            id: _contentPane
            Layout.fillHeight: true
            Layout.fillWidth: true
            background: null
        }
    }
}

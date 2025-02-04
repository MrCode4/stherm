import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm
import Ronia

/*! ***********************************************************************************************
 * BusyPopUp: Displays a Busy indicator in a popup non user closable
 * ************************************************************************************************/
I_PopUp {
    id: root

    closePolicy: I_PopUp.NoAutoClose
    topPadding: 24
    bottomPadding: 24
    horizontalPadding: 24
    title: ""

    contentItem: ColumnLayout {
        spacing: root.spacing

        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            Layout.fillWidth: true
            Layout.leftMargin: parent.labelMargin
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.MarkdownText
            font.pointSize: Application.font.pointSize
            text: title
            elide: Text.ElideRight
            color: enabled ? Style.foreground : Style.hintTextColor
            linkColor: Style.linkColor
            visible: title.length > 0
        }

    }
}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleRepeatePage provides ui for setting repeats in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: Math.max(_contentLay.implicitWidth, implicitHeaderWidth) + leftPadding + rightPadding
    implicitHeight: _contentLay.implicitHeight + implicitHeaderHeight + implicitFooterHeight + topPadding + bottomPadding
    topPadding: 24
    font.bold: true
    title: "Repeat"
    backButtonVisible: false
    titleHeadeingLevel: 3

    /* Children
     * ****************************************************************************************/
    GridLayout {
        id: _contentLay
        anchors.fill: parent

        columns: 7

        Label { Layout.alignment: Qt.AlignCenter; text: "Mu" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Tu" }
        Label { Layout.alignment: Qt.AlignCenter; text: "We" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Th" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Fr" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Sa" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Su" }

        RadioButton {id: _muBtn; autoExclusive: false; checked: true }
        RadioButton {id: _tuBtn; autoExclusive: false }
        RadioButton {id: _weBtn; autoExclusive: false }
        RadioButton {id: _thBtn; autoExclusive: false }
        RadioButton {id: _frBtn; autoExclusive: false }
        RadioButton {id: _saBtn; autoExclusive: false }
        RadioButton {id: _suBtn; autoExclusive: false }
    }
}

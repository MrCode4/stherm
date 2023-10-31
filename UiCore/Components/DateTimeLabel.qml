import QtQuick
import QtQuick.Layouts
import QtQml

import Ronia
import Stherm

/*! ***********************************************************************************************
 * DateTimeLabel visualizes current date and time
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! The maximum with that is required by DateTimeLabel
    property real   maximumWidth: _fontMetric.boundingRect("00:00 AM").width + leftPadding + rightPadding

    //!
    property bool   is12Hour:   false

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: _dateTimeCol.implicitWidth + leftPadding + rightPadding
    implicitHeight: _dateTimeCol.implicitHeight + topPadding + bottomPadding
    padding: 4
    background: null

    /* Childrent
     * ****************************************************************************************/
    ColumnLayout {
        id: _dateTimeCol

        anchors.centerIn: parent
        spacing: 4

        //! Time Label
        Label {
            id: _timeLbl
            Layout.alignment: Qt.AlignHCenter
            font {
                family: "monospace"
                pointSize: Qt.application.font.pointSize * 1.4
            }
            text: "00:00"
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: "grey"
        }

        //! Date Label
        Label {
            id: _dateLbl
            Layout.fillWidth: true
            opacity: 0.75
            font.pointSize: Application.font.pointSize * 0.8
            horizontalAlignment: "AlignHCenter"
        }
    }

    //! Timer to update date and time
    Timer {
        interval: 100
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: {
            var now = new Date();
            _timeLbl.text = now.toLocaleTimeString(locale, is12Hour ? "hh:mm AP" : "hh:mm")
            _dateLbl.text = now.toLocaleDateString(locale, "MMM dd ddd")
        }
    }

    FontMetrics {
        id: _fontMetric
        font: _timeLbl.font
    }
}

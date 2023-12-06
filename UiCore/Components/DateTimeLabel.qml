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

    //! Display date or not
    property bool   showDate: true

    /* Object properties
     * ****************************************************************************************/
    width: 200
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
        Row {
            Layout.alignment: Qt.AlignCenter

            Label {
                id: _timeLbl
                font.pointSize: _root.font.pointSize * 1.4
                text: "00:00"
            }

            Label {
                id: amPmLbl
                y: parent.height - height
                visible: is12Hour
                font.pointSize: _root.font.pointSize * 0.8
            }
        }

        Rectangle {
            Layout.fillWidth: true
            visible: showDate
            implicitHeight: 1
            color: "grey"
        }

        //! Date Label
        Label {
            id: _dateLbl
            Layout.fillWidth: true
            visible: showDate
            opacity: 0.75
            font.pointSize: Application.font.pointSize * 0.8
            horizontalAlignment: "AlignHCenter"
        }
    }

    Connections {
        target: DateTimeManager

        function onNowChanged()
        {
            var now = DateTimeManager.now;
            var currentTime = now.toLocaleTimeString(locale, is12Hour ? "hh:mm AP" : "hh:mm");
            if (is12Hour) {
                _timeLbl.text = currentTime.slice(0, currentTime.length - 3);
                amPmLbl.text = currentTime.slice(currentTime.length - 3, currentTime.length);
            } else {
                _timeLbl.text = currentTime;
            }

            if (showDate) {
                _dateLbl.text = now.toLocaleDateString(locale, "MMM dd ddd");
            }
        }
    }

    FontMetrics {
        id: _fontMetric
        font: _timeLbl.font
    }
}

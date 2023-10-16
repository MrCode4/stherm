import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQml

import Stherm

/*! ***********************************************************************************************
 * DateTimeLabel visualizes current date and time
 * ***********************************************************************************************/
Control {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: _dateTimeCol.implicitWidth + leftPadding + rightPadding
    implicitHeight: _dateTimeCol.implicitHeight + topPadding + bottomPadding
    leftPadding: AppStyle.size / 60
    rightPadding: AppStyle.size / 60
    topPadding:AppStyle.size / 60 / 2
    bottomPadding: AppStyle.size / 60 / 2
    background: null

    /* Childrent
     * ****************************************************************************************/
    ColumnLayout {
        id: _dateTimeCol

        anchors.fill: parent
        spacing: 2

        //! Time Label
        Label {
            id: _timeLbl
            Layout.alignment: Qt.AlignHCenter
            font {
                family: "monospace"
                pixelSize: AppStyle.size / 20
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
            horizontalAlignment: "AlignHCenter"
            font {
                pixelSize: AppStyle.size / 30
            }
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
            _timeLbl.text = now.toLocaleTimeString(locale, "hh:mm")
            _dateLbl.text = now.toLocaleDateString(locale, "MMMM dd ddd")
        }
    }
}

import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SplashScreen
 * ***********************************************************************************************/
ApplicationWindow {
    id: _root

    /* Signals
     * ****************************************************************************************/
    signal ready();

    /* Object properties
     * ****************************************************************************************/
    width: AppStyle.size
    height: AppStyle.size
    visible: false
    flags: Qt.SplashScreen
    font.capitalization: "AllUppercase"
    font.letterSpacing: 4

    /* Children
     * ****************************************************************************************/
    Label {
        id: _loadingLbl
        anchors {
            centerIn: parent
            horizontalCenterOffset: -20
        }
        textFormat: "MarkdownText"
        text: "## Loading"
    }

    Label {
        id: _dotsLbl
        anchors {
            left: _loadingLbl.right
            verticalCenter: _loadingLbl.verticalCenter
            leftMargin: 6
        }
        textFormat: "MarkdownText"
        text: "## . . ."
    }

    SequentialAnimation {
        running: true
        loops: Animation.Infinite

        PropertyAnimation {
            target: _dotsLbl
            property: "text"
            to: ""
            duration: 300
        }

        PropertyAnimation {
            target: _dotsLbl
            property: "text"
            to: "## ."
            duration: 300
        }

        PropertyAnimation {
            target: _dotsLbl
            property: "text"
            to: "## . ."
            duration: 300
        }

        PropertyAnimation {
            target: _dotsLbl
            property: "text"
            to: "## . . ."
            duration: 300
        }

        PauseAnimation { duration: 300 }
    }

    Timer {
        interval: 4000
        running: true
        repeat: false
        onTriggered: {
            ready();
        }
    }
}

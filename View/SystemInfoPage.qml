import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * SystemInfoPage retrieves device info and displays them
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property delcaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Info"

    /* Children
     * ****************************************************************************************/
    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: _infoLabel.implicitWidth
        contentHeight: _infoLabel.implicitHeight
        boundsBehavior: Flickable.StopAtBounds

        Label {
            id: _infoLabel
            lineHeight: 1.2
            wrapMode: Text.WordWrap
            text: JSON.stringify(deviceInfo, null, 4)
        }
    }
}

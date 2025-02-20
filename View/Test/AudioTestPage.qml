import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AudioTestPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Object properties
     * ****************************************************************************************/    
    title: "Audio Test"
    useSimpleStackView: true

    /* Children
     * ****************************************************************************************/
    //! Next button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            //! Next page
            gotoPage("qrc:/Stherm/View/Test/InternalSensorTestPage.qml", {"uiSession": uiSession});
        }
    }

    //! Sound play button
    ToolButton {
        anchors.centerIn: parent
        contentItem: RoniaTextIcon {
            font.pointSize: Style.fontIconSize.largePt * 2
            text: FAIcons.circlePlay
        }

        onClicked: {
            //! Play a sound
        }
    }
}

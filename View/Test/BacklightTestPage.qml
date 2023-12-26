import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * BacklightTestPage
 * ***********************************************************************************************/
BacklightPage {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    topPadding: 0
    bottomPadding: 32 * scaleFactor
    title: "Backlight Test"
    isTest: true
    // hasShades: false

    // onUnshadedColorChanged: {
        //! Apply selected color to device immediately
    // }

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
            if (_root.StackView.view) {
                _root.StackView.view.push("qrc:/Stherm/View/Test/AudioTestPage.qml", {
                                              "uiSession": uiSession
                                          })
            }

            // Revert when change backlight with go back in test mode
            if (isTest) {
                revertToModel();
            }
        }
    }
}

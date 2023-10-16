import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * BasePageView is the basic class for all pages
 * ***********************************************************************************************/
Page {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to app UiSession since most pages need this
    property UiSession              uiSession

    //! Ref to I_DeviceController
    property I_DeviceController     deviceController

    //! This should hold a callback to be called when back button is clicked.
    property var                    backButtonCallback

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 480
    implicitHeight: 480
    leftPadding: 8
    rightPadding: 8
    topPadding: 8
    bottomPadding: 8
    header: RowLayout {
        ToolButton {
            contentItem: RoniaTextIcon {
                text: "\uf060"
            }

            onClicked: if (backButtonCallback instanceof Function) backButtonCallback();
        }

        Label {
            Layout.fillHeight: true
            Layout.fillWidth: true
            textFormat: "MarkdownText"
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
            text: `## ${title}`
            elide: "ElideRight"
        }
    }

    //! By default if Page is inside a StackView it will be popped if not nothing happens.
    //! For most Pages this is enough, although it might be neccessary to override it for some
    backButtonCallback: function() {
        if (_root.StackView.view) {
            //! Then Page is inside an StackView
            if (_root.StackView.view.currentItem == _root) {
                _root.StackView.view.pop();
            }
        }
    }
}

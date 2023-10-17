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
    property I_DeviceController     deviceController: uiSession?.deviceController ?? null

    //! This should hold a callback to be called when back button is clicked.
    property var                    backButtonCallback

    //! Controls visibility of backbutton
    property bool                   backButtonVisible: true

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size
    leftPadding: AppStyle.size / 60
    rightPadding: AppStyle.size / 60
    topPadding: AppStyle.size / 60
    bottomPadding: AppStyle.size / 60
    header: RowLayout {
        ToolButton {
            visible: backButtonVisible
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

import QtQuick
import QtQuick.Layouts

import Ronia
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

    //! App model
    property I_Device               appModel: uiSession?.appModel ?? null

    //! Ref to I_DeviceController
    property I_DeviceController     deviceController: uiSession?.deviceController ?? null

    //! Page title heading level: from 1 to 6
    property int                    titleHeadeingLevel: 3

    //! This should hold a callback to be called when back button is clicked.
    property var                    backButtonCallback

    //! FontAwesome text icon of backbutton
    property string                 backButtonTextIcon: "\uf060"

    //! Controls visibility of backbutton
    property bool                   backButtonVisible: true

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size
    leftPadding: 0
    rightPadding: 0
    topPadding: 8 * scaleFactor
    bottomPadding: 0
    header: Control {
        horizontalPadding: 0
        verticalPadding: 0
        background: null
        contentItem: RowLayout {
            ToolButton {
                visible: backButtonVisible
                contentItem: RoniaTextIcon {
                    text: backButtonTextIcon
                }

                onClicked: if (backButtonCallback instanceof Function) backButtonCallback();
            }

            Label {
                Layout.fillHeight: true
                Layout.fillWidth: true
                //! If there is only the back button and the title in the header, right-margin is
                //! to make the header title look centered
                Layout.rightMargin: parent.children.length === 2 && backButtonVisible
                                    ? parent.children[0].implicitWidth + parent.spacing : 0

                visible: title.length > 0
                textFormat: "MarkdownText"
                verticalAlignment: "AlignVCenter"
                horizontalAlignment: "AlignHCenter"
                text: `${"#".repeat(Math.max(1, Math.min(6, titleHeadeingLevel)))} ${title}`
                elide: "ElideRight"
            }
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

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * BasePageView is the basic class for all pages
 * ***********************************************************************************************/
Page {
    id: root

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
    property string                 backButtonTextIcon: FAIcons.arrowLeft

    //! Controls visibility of backbutton
    property bool                   backButtonVisible: true

    property bool                   backButtonEnabled: true

    property color                  headerColor: "white"

    property bool useSimpleStackView: false

    property bool enableTitleTap: false
    property int titleLongTapInterval: 5000

    signal titleTapped()
    signal titleLongTapped()

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size
    leftPadding: 8 * scaleFactor
    rightPadding: 8 * scaleFactor
    topPadding: 8 * scaleFactor
    bottomPadding: 4 * scaleFactor
    header: Control {
        horizontalPadding: 2
        verticalPadding: 0
        background: null
        contentItem: RowLayout {
            ToolButton {
                visible: backButtonVisible
                enabled: backButtonEnabled
                contentItem: RoniaTextIcon {
                    text: backButtonTextIcon
                    color: enabled ? root.headerColor : Style.hintTextColor
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
                color: root.headerColor
                verticalAlignment: "AlignVCenter"
                horizontalAlignment: "AlignHCenter"
                text: `${"#".repeat(Math.max(1, Math.min(6, titleHeadeingLevel)))} ${title}`
                elide: "ElideRight"

                MouseArea {
                    anchors.fill: parent
                    enabled: root.enableTitleTap
                    pressAndHoldInterval: root.titleLongTapInterval
                    onClicked: root.titleTapped()
                    onPressAndHold: root.titleLongTapped()
                }
            }
        }
    }

    //! By default if Page is inside a StackView it will be popped if not nothing happens.
    //! For most Pages this is enough, although it might be neccessary to override it for some    
    backButtonCallback: tryGoBack

    function tryGoBack() {
        if (useSimpleStackView) {
            if (testsStackView.currentItem == root) {
                testsStackView.pop();
            }
        }
        else if (root.StackView.view) {
            //! Then Page is inside an StackView
            if (root.StackView.view.currentItem === root) {
                root.StackView.view.pop();
            }
        }
    }

    function gotoPage(page, props)  {
        if (useSimpleStackView) {
            testsStackView.push(page, props);
        }
        else if (root.StackView.view)  {
            root.StackView.view.push(page, props);
        }
    }
}

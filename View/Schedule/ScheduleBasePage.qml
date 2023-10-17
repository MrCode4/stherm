import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * ScheduleBasePage is the basic class for pages inside AddSchedulePage
 * ***********************************************************************************************/
Page {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! This should hold a callback to be called when back button is clicked.
    property var                    backButtonCallback

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                            + implicitFooterHeight + topPadding + bottomPadding
                                          : 0
    leftPadding: AppStyle.size / 60
    rightPadding: AppStyle.size / 60
    topPadding: 24
    bottomPadding: 24
    header: RowLayout {
        Label {
            Layout.fillHeight: true
            Layout.fillWidth: true
            textFormat: "MarkdownText"
            text: `### ${title}`
            elide: "ElideRight"
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
        }
    }
    footer: RowLayout {
        ToolButton {
            visible: _root.StackView.view.depth > 1
            contentItem: RoniaTextIcon {
                text: "\uf060"
            }

            onClicked: if (backButtonCallback instanceof Function) backButtonCallback();
        }
    }

    //! Go back to prev page if in Stackview
    backButtonCallback: function() {
        if (_root.StackView.view) {
            //! Then Page is inside an StackView
            if (_root.StackView.view.currentItem == _root) {
                _root.StackView.view.pop();
            }
        }
    }
}

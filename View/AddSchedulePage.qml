import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Add New Schedule"
    footer: RowLayout {
        ToolButton {
            visible: _newSchedulePages.depth > 1
            contentItem: RoniaTextIcon {
                text: "\uf060"
            }

            onClicked: {
                _newSchedulePages.pop();
            }
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Next/Confirm button
    ToolButton {
        parent: _root.header

        RoniaTextIcon {
            anchors.centerIn: parent
            opacity: _newSchedulePages.currentItem?.nextPage ? 1. : 0.
            text: "\uf061"
        }

        RoniaTextIcon {
            opacity: _newSchedulePages.currentItem?.nextPage ? 0. : 1.
            anchors.centerIn: parent
            text: "\uf00c"
        }

        onClicked: {
            if (!_newSchedulePages.currentItem.nextPage) {
                //! It's done, save schedule
                console.log('Save Schedule');
            } else {
                //! Go to next page
                _newSchedulePages.push(_newSchedulePages.currentItem.nextPage)
            }
        }
    }

    //! StackView for new-schedule pages
    StackView {
        id: _newSchedulePages
        anchors.centerIn: parent
        implicitHeight: currentItem?.implicitHeight
        implicitWidth: currentItem?.implicitWidth

        initialItem: _typePage
    }

    //! Page Components
    Component {
        id: _typePage

        ScheduleTypePage {
            readonly property Component nextPage: _tempraturePage
        }
    }

    Component {
        id: _tempraturePage

        ScheduleTempraturePage {
            readonly property Component nextPage: _startTimePage
        }
    }

    Component {
        id: _startTimePage

        Label {
            readonly property Component nextPage: _endTimePage

            textFormat: "MarkdownText"
            text: "# Start Time"
        }
    }

    Component {
        id: _endTimePage

        Label {
            readonly property Component nextPage: _repeatPage

            textFormat: "MarkdownText"
            text: "# End Time"
        }
    }

    Component {
        id: _repeatPage

        Label {
            readonly property Component nextPage: _preivewPage

            textFormat: "MarkdownText"
            text: "# Repeat"
        }
    }

    Component {
        id: _preivewPage

        Label {
            readonly property Component nextPage: null

            textFormat: "MarkdownText"
            text: "# New Schedule Preview"
        }
    }
}

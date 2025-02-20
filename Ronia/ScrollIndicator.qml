import QtQuick
import QtQuick.Templates as T

import Ronia

T.ScrollIndicator {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 2

    contentItem: Rectangle {
        implicitWidth: 4
        implicitHeight: 4
        visible: control.size < 1.0
        opacity: 0.8 //! ScrollIndicator is always on
        color: control.Material.scrollBarColor
        radius: 2

        states: State {
            name: "active"
            when: control.active

            PropertyChanges {
                target: control.contentItem
                opacity: 1
                color: Qt.alpha(control.Material.scrollBarColor, 0.5)
            }
        }

        transitions: [
            Transition {
                to: "active"

                ParallelAnimation {
                    ColorAnimation { target: control.contentItem; duration: 250; property: "color" }
                }
            },

            Transition {
                from: "active"

                SequentialAnimation {
                    PauseAnimation { duration: 400 }
                    ParallelAnimation {
                        NumberAnimation { target: control.contentItem; duration: 300; property: "opacity" }
                        ColorAnimation { target: control.contentItem; duration: 300; property: "color" }
                    }
                }
            }
        ]
    }
}

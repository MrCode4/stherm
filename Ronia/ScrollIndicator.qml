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
    }
}

import QtQuick
import QtQuick.Controls.impl
import QtQuick.Templates as T

import Ronia

T.Tumbler {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    delegate: Text {
        text: modelData
        color: control.Material.foreground
        font: control.font
        opacity: (1.0 - Math.abs(Tumbler.displacement) / (control.visibleItemCount / 2)) * (control.enabled ? 1 : 0.6)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        height: control.contentItem.delegateHeight

        required property var modelData
        required property int index
    }

    contentItem: ListView {
        property int visibleIndex: count ? Math.max(0, Math.min(Math.round((contentY + topMargin) / delegateHeight)
                                                                , count - 1)
                                                    )
                                         : 0

        implicitWidth: 60
        implicitHeight: 200
        spacing: 0
        model: control.model
        delegate: control.delegate
        preferredHighlightBegin: delegateHeight * 2
        topMargin: delegateHeight * 2
        bottomMargin: delegateHeight * 2
        snapMode: ListView.SnapToItem
        currentIndex: visibleIndex

        property real delegateHeight: control.availableHeight / control.visibleItemCount

        onVisibleIndexChanged: {
            if (currentIndex !== visibleIndex) {
                currentIndex = visibleIndex;
            }
        }

        Connections {
            target: control

            function onCurrentIndexChanged()
            {
                if (control.contentItem.visibleIndex !== control.currentIndex) {
                    control.contentItem.positionViewAtIndex(control.currentIndex, ListView.Center);
                }
            }
        }
    }
}

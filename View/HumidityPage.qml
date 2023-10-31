import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * HumidityPage is a page for controlling desired humidity
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! I_Device
    readonly property I_Device  device: uiSession?.appModel ?? null

    //! Humidity
    readonly property alias     humidity: _humSlider.value

    /* Object properties
     * ****************************************************************************************/
    title: "Humidity Control"
    leftPadding: AppStyle.size / 12
    rightPadding: AppStyle.size / 12

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Apply humidity in backend
            if (device) {
                device.requestedHum = humidity;

                //! Update requested humidity to device
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: _root.availableWidth

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: "Please select value for the humidity"
        }

        TickedSlider {
            id: _humSlider
            Layout.fillWidth: true
            from: 20
            to: 70
            ticksCount: 50
            majorTickCount: 5
            stepSize: 1.
            valueChangeAnimation: true
        }

        ToolTip {
            parent: _humSlider.handle
            y: -height - AppStyle.size / 30
            x: (parent.width - width) / 2
            visible: _humSlider.pressed
            timeout: Number.MAX_VALUE
            delay: 0
            text: Number(_humSlider.value).toLocaleString(locale, "f", 0)
        }
    }
}

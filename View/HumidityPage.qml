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

    //! SystemAccessories
    property SystemAccessories systemAccessories: appModel.systemSetup.systemAccessories

    //! Humidity
    readonly property alias     humidity: _humSlider.value

    /* Object properties
     * ****************************************************************************************/
    title: "Humidity Control"

    Component.onCompleted: deviceController.updateEditMode(AppSpec.EMRequestedHumidity);

    Component.onDestruction: deviceController.updateEditMode(AppSpec.EMNone);

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            if (deviceController) {
                deviceController.setRequestedHumidity(humidity)
            }

            deviceController.pushSettings();

            //! Also move out of this Page
            if (_root.StackView.view && _root.StackView.view.depth > 1
                    && _root.StackView.view.currentItem === _root) {
                _root.StackView.view.pop();
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: "Please select value for the humidity"
        }

        RowLayout {
            spacing: 0

            Label {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: height / 2
                font.pointSize: _root.font.pointSize * 0.9
                text: "%"
            }

            TickedSlider {
                id: _humSlider
                Layout.fillWidth: true
                from: 20
                to: 70
                value: device?.requestedHum ?? 0
                ticksCount: 50
                majorTickCount: 5
                stepSize: 1.
                valueChangeAnimation: true
            }
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

        //! Spacer
        Item {
            Layout.preferredWidth: 20
            Layout.preferredHeight: 25
        }

        Label {
            Layout.leftMargin: 25
            Layout.alignment: Qt.AlignLeft
            text: systemAccessories.accessoriesType === AppSpecCPP.Humidifier ? "Humidifier" : "Dehumidifier";
        }
    }
}

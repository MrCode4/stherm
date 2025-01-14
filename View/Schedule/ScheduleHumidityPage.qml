import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * HumidityPage is a page for controlling desired humidity
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! I_Device
    readonly property I_Device  device: uiSession?.appModel ?? null

    //! SystemAccessories
    property SystemAccessories systemAccessories: device.systemSetup.systemAccessories

    //! Humidity
    readonly property alias     humidity: _humSlider.value

    //! Humidity is alwasy valid
    readonly property bool  isValid: true

    property bool editMode: false

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property ScheduleCPP    schedule

    /* Object properties
     * ****************************************************************************************/
    title: "Humidity Control"
    backButtonVisible: false
    titleHeadeingLevel: 4

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing
    ToolButton {
        parent: schedule ? root.header.contentItem : root
        visible: editMode
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            schedule.humidity = humidity;
            backButtonCallback();
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
                font.pointSize: root.font.pointSize * 0.9
                text: "%"
            }

            TickedSlider {
                id: _humSlider
                Layout.fillWidth: true
                from: 20
                to: 70
                value: schedule?.humidity ?? 0
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
            text: {
                if ((device?.systemSetup?.systemAccessories?.accessoriesWireType ?? AppSpecCPP.None) === AppSpecCPP.None) {
                    return "No Accessories"
                }

                if (systemAccessories.accessoriesType === AppSpec.Humidifier)
                    return "Humidifier";
                else if (systemAccessories.accessoriesType === AppSpec.Dehumidifier)
                    return "Dehumidifier";


                return "No Accessories"
            }

        }
    }
}

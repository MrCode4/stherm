import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SelectTimezonePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal timezoneSelected(string timezoneId)

    /* Object properties
     * ****************************************************************************************/
    topPadding: 0
    title: "Select Time Zone"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            TextField {
                id: searchTf

                property var regexp: new RegExp(`.*${text}.*`, "i")

                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.fillWidth: true
            }

            DateTimeLabel {
                Layout.topMargin: 12
                font.pointSize: root.font.pointSize * 0.8
                showDate: false
            }
        }

        ListView {
            readonly property var timezones: DateTimeManager.timezones()

            Layout.fillHeight: true
            Layout.fillWidth: true

            clip: true
            model: searchTf.length > 0 ? timezones.filter((element, index) => element.id.toString().match(searchTf.regexp))
                                       : timezones

            delegate: ItemDelegate {
                width: ListView.view.width
                height: Style.delegateHeight

                contentItem: RowLayout {
                    ColumnLayout {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        spacing: 4
                        Label {
                            Layout.fillWidth: true
                            text: modelData.id
                        }

                        Label {
                            Layout.fillWidth: true
                            opacity: 0.7
                            font.pointSize: root.font.pointSize * 0.8
                            text: modelData.city
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        opacity: 0.7
                        font.pointSize: root.font.pointSize * 0.8
                        text: modelData.offset
                    }
                }

                onClicked: timezoneSelected(modelData.id)
            }
        }
    }
}

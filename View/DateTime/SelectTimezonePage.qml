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
            id: timezonesLv
            readonly property var timezones: DateTimeManager.timezones()

            Layout.fillHeight: true
            Layout.fillWidth: true
            ScrollIndicator.vertical: ScrollIndicator { }

            clip: true
            model: searchTf.length > 0 ? timezones.filter((element, index) => element.id.toString().match(searchTf.regexp))
                                       : timezones

            delegate: ItemDelegate {
                id: tzDelegate

                required property var modelData
                required property int index
                property bool current: modelData.id === DateTimeManager.currentTimeZone.id

                width: ListView.view.width - 12
                height: Style.delegateHeight

                contentItem: RowLayout {
                    ColumnLayout {
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        spacing: 4

                        Label {
                            Layout.fillWidth: true
                            text: modelData.id
                            color: tzDelegate.current ? Style.linkColor : Style.foreground
                        }

                        Label {
                            Layout.fillWidth: true
                            opacity: 0.7
                            font.pointSize: root.font.pointSize * 0.8
                            text: modelData.city
                            color: tzDelegate.current ? Style.linkColor : Style.foreground
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        opacity: 0.7
                        font.pointSize: root.font.pointSize * 0.8
                        text: modelData.offset
                        color: tzDelegate.current ? Style.linkColor : Style.foreground
                    }
                }

                onClicked: timezoneSelected(modelData.id)
            }
        }
    }

    Component.onCompleted: positionViewAtCurrentTimeZone()

    /* Methods
     * ****************************************************************************************/
    function positionViewAtCurrentTimeZone()
    {
        var currentTzId = DateTimeManager.currentTimeZone.id.toString();
        var currentTzIndex = timezonesLv.model.findIndex((element) => {
                                                             return element.id === currentTzId;
                                                         })
        timezonesLv.positionViewAtIndex(currentTzIndex, ListView.Center);
    }
}

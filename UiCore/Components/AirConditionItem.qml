import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AirConditionItem
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Condition of air
    property int    condition:  0

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: _mainRow.implicitWidth + leftPadding + rightPadding
    implicitHeight: _mainRow.implicitHeight + topPadding + bottomPadding
    leftPadding: AppStyle.size / 60
    rightPadding: AppStyle.size / 60
    topPadding: AppStyle.size / 60 / 2
    bottomPadding: AppStyle.size / 60 / 2
    background: null

    /* Children
     * ****************************************************************************************/
    RowLayout {
        id: _mainRow
        //! Condition image
        Item {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillHeight: true
            implicitWidth: _conditionIcon.implicitWidth

            Image {
                readonly property var sources: [
                    "qrc:/Stherm/Images/condition-good.png",
                    "qrc:/Stherm/Images/condition-moderate.png",
                    "qrc:/Stherm/Images/condition-poor.png"
                ]

                id: _conditionIcon
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: condition >= 0 && condition < sources.length ? sources[condition] : "qrc:/Stherm/Images/condition-good.png"
            }
        }

        Label {
            readonly property var conditionNames: [ "Good", "Fair", "Poor" ]

            Layout.fillWidth: true
            font.pointSize: Qt.application.font.pointSize * 1.25
            text: condition >= 0 && condition < conditionNames.length ? conditionNames[condition] : "Good"
        }
    }
}

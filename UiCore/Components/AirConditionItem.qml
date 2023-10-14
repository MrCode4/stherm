import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

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
    leftPadding: 8
    rightPadding: 8
    topPadding: 4
    bottomPadding: 4
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
                source: 0 < condition && condition < sources.length ? sources[condition] : "qrc:/Stherm/Images/condition-good.png"
            }
        }

        Label {
            readonly property var conditionNames: [ "Good", "Moderate", "Poor" ]

            Layout.fillWidth: true
            font.pixelSize: 22
            text: 0 < condition && condition < conditionNames.length ? conditionNames[condition] : "Good"
        }
    }
}

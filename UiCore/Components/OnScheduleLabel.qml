import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

Control {
    id: root

    /*! Object properties
     * ****************************************************************************************/
    leftPadding: 4
    rightPadding: 4
    topPadding: 2
    bottomPadding: 2
    contentItem: mainCol

    /*! Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainCol

        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 1

            //! Power off icon
            RoniaTextIcon {
                font.pointSize: Style.fontIconSize.smallPt
                text: "\uf011" //! power-off icon
                color: Style.green
            }

            Label {
                font.weight: 900
                text: "N"
                color: Style.green
            }
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "Schedule"
        }
    }
}

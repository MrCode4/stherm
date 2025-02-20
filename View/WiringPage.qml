import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WiringPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Wiring model
    property Wiring     wiring: appModel?.wiring ?? null

    /* Object properties
     * ****************************************************************************************/
    leftPadding: width * 0.1
    rightPadding: leftPadding
    title: "Wiring"

    /* Children
     * ****************************************************************************************/
    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            id: _firstColLay

            CheckBox {
                hoverEnabled: false
                text: "R"
                checked: root.wiring?.isR ?? false
                onCheckedChanged: root.wiring.isR = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "C"
                checked: root.wiring?.isC ?? false
                onCheckedChanged: root.wiring.isC = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "G"
                checked: root.wiring?.isG ?? false
                onCheckedChanged: root.wiring.isG = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "Y1"
                checked: root.wiring?.isY1 ?? false
                onCheckedChanged: root.wiring.isY1 = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "Y2"
                checked: root.wiring?.isY2 ?? false
                onCheckedChanged: root.wiring.isY2 = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "T2"
                checked: root.wiring?.isT2 ?? false
                onCheckedChanged: root.wiring.isT2 = checked
            }
        }

        ColumnLayout {
            id: _secondColLay
            Layout.alignment: Qt.AlignRight

            CheckBox {
                hoverEnabled: false
                text: "W1"
                checked: root.wiring?.isW1 ?? false
                onCheckedChanged: root.wiring.isW1 = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "W2"
                checked: root.wiring?.isW2 ?? false
                onCheckedChanged: root.wiring.isW2 = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "W3"
                checked: root.wiring?.isW3 ?? false
                onCheckedChanged: root.wiring.isW3 = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "O/B"
                checked: root.wiring?.isOB ?? false
                onCheckedChanged: root.wiring.isOB = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "T1P"
                checked: root.wiring?.isT1p ?? false
                onCheckedChanged: root.wiring.isT1p = checked
            }

            CheckBox {
                hoverEnabled: false
                text: "T1N"
                checked: root.wiring?.isT1n ?? false
                onCheckedChanged: root.wiring.isT1n = checked
            }
        }
    }
}

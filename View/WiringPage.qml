import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WiringPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

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
                checked: wiring?.isR ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "C"
                checked: wiring?.isC ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "G"
                checked: wiring?.isG ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "Y1"
                checked: wiring?.isY1 ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "Y2"
                checked: wiring?.isY2 ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "T2"
                checked: wiring?.isT2 ?? false
            }
        }

        ColumnLayout {
            id: _secondColLay
            Layout.alignment: Qt.AlignRight

            CheckBox {
                hoverEnabled: false
                text: "W1"
                checked: wiring?.isW1 ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "W2"
                checked: wiring?.isW2 ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "W3"
                checked: wiring?.isW3 ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "O/B"
                checked: wiring?.isOB ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "T1P"
                checked: wiring?.isT1p ?? false
            }

            CheckBox {
                hoverEnabled: false
                text: "T1N"
                checked: wiring?.isT1n ?? false
            }
        }
    }

    //! To make CheckBoxes non-editable
    Pane {
        anchors.fill: parent
        background: null
        hoverEnabled: false
    }
}

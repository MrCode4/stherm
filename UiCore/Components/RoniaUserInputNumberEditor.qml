import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QTemplateProject

/*! ***********************************************************************************************
 * User Input Component
 * ************************************************************************************************/
RoniaUserInputBase {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property real   minimum:    Number.MAX_SAFE_INTEGER * -1

    property real   maximum:    Number.MAX_SAFE_INTEGER

    property alias  value:      numberEditor.value

    property real   initialValue: 0

    property int    stepSize:   1

    property bool   doubleStep: false

    property alias  numberEditor: numberEditor



    /* Object Properties
     * ****************************************************************************************/
    opacity: enabled ? 1 : 0.6

    /* Signals
     * ****************************************************************************************/
    signal edittingFinished()

    /* Children
     * ****************************************************************************************/

    //! Number Editor Box
    editor: Rectangle {
        width: 110
        height: 26
        color: AppStyle.inputBoxGray
        radius: 2

        Item {
            width: 0.15 * parent.width
            height: parent.height
            anchors.left: parent.left
            RoniaTextIcon {
                text: "\uf0d9"
                color: AppStyle.primaryTextColor
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                scale: mouseArea1.containsMouse ? 1.2 : 1
            }
            MouseArea {
                id: mouseArea1
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    numberEditor.decrement();
                    control.edittingFinished();
                }
            }
        }

        //! Number Editor Component
        RoniaNumberEditor {
            id: numberEditor
            width: 0.70 * parent.width
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            from: control.minimum
            to: control.maximum
            initialValue: initialValue
            stepSize: control.stepSize
            doubleStep:control.doubleStep
            color: AppStyle.primaryTextColor
            backgroundColor: Qt.lighter(AppStyle.inputBoxGray, 1.1)
            onEditingFinished: control.edittingFinished()
        }

        Item {
            width: 0.15 * parent.width
            height: parent.height
            anchors.right: parent.right
            RoniaTextIcon {
                text: "\uf0da"
                color: AppStyle.primaryTextColor
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                scale: mouseArea2.containsMouse ? 1.2 : 1
            }
            MouseArea {
                id: mouseArea2
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    numberEditor.increment();
                    control.edittingFinished();
                }
            }
        }
    }
}

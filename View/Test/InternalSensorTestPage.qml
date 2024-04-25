import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InternalSensorTestPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    property var model: ({})

    title: "Internal Sensor Test"

    Component.onCompleted: {
        root.model = deviceController.getTestData();
    }

    /* Children
     * ****************************************************************************************/
    //! Next button
    ToolButton {
        id: nextBtn
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            if (deviceController.startMode !== 0) {
                //! Next page
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/Test/RelayTestPage.qml", {
                                                 "uiSession": uiSession
                                             })
                }
            } else {
                //! Test mode enabled with GPIO as there is no ti board connected
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/Test/QRCodeTestPage.qml", {
                                                  "uiSession": uiSession
                                              })
                }
            }
        }
    }

    //! override button
    ToolButton {
        id: overrideBtn
        contentItem: RoniaTextIcon {
            text: FAIcons.penCircle
        }
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 5
        checkable: true
        checked: false
        onToggled: {
            if (checked)
                temperatureField.forceActiveFocus()
            else
                deviceController.setTestData(model.temperature, true)
        }
    }

    //! reset button
    ToolButton {
        id: resetBtn
        contentItem: RoniaTextIcon {
            text: FAIcons.revert
        }
        anchors.top: overrideBtn.bottom
        anchors.right: overrideBtn.right
        anchors.topMargin: 5
        visible: !overrideBtn.checked
        checkable: false
        onClicked: {
            deviceController.setTestData(model.temperature, false)
        }
    }

    //! timer for updating values
    Timer {
        interval: 1000
        repeat: true
        running: !overrideBtn.checked
        onTriggered: {
          root.model = deviceController.getTestData();
        }
    }

    //! items
    GridLayout {
        anchors.fill: parent
        columns: 2

        Label {
            text: "Temperature :"
        }

        TextField {
            id: temperatureField
            readOnly: !overrideBtn.checked
            Layout.preferredHeight: 50
            text: readOnly ? (root.model?.temperature.toFixed(3)  ?? "") : text

            validator: DoubleValidator {
                top: 40
                bottom: 0
                decimals: 1
            }

            onEditingFinished: {
                if (acceptableInput)
                    model.temperature = text
            }
        }

        Label {
            text: "Humidity :"
        }

        Label {
            text:root.model?.humidity  ?? "NaN"
            Layout.preferredWidth:  temperatureField.width
        }

        Label {
            text: "TOF :"
        }

        Label {
            text: (root.model?.RangeMilliMeter > 0) ? ("ON (" + root.model.RangeMilliMeter + ")") : "OFF"
            Layout.preferredWidth:  temperatureField.width
        }

        Label {
            text: "Ambient :"
        }

        Label {
            text:root.model?.brighness  ?? ""
            Layout.preferredWidth:  temperatureField.width
        }

        Label {
            text: "CO2 :"
        }

        Label {
            text:root.model?.iaq  ?? ""
            Layout.preferredWidth:  temperatureField.width
        }

        Label {
            text: "Fan Speed:"
        }

        Label {
            text:root.model?.fanSpeed  ?? ""
            Layout.preferredWidth:  temperatureField.width
        }
    }
}

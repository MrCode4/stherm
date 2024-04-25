import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * TestFailedPopup
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Error text
    property string errorMessage: ""

    /* Signal declaration
     * ****************************************************************************************/
    signal retryClicked
    signal continueClicked

    /* Object properties
     * ****************************************************************************************/
    title: "Test failed"
    closeButtonVisible: false
    closePolicy: Popup.NoAutoClose

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.fillWidth: true
            textFormat: "MarkdownText"
            text: errorMessage
            wrapMode: "Wrap"
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
        }

        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 32

            Button {
                text: "Retry"
                onClicked: {
                    root.close()
                    root.retryClicked()
                }
            }

            Button {
                text: "Continue"
                onClicked: {
                    root.close()
                    root.continueClicked()
                }
            }
        }
    }
}

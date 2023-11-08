import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * I_Dialog is the base calss for any prompt dialog (which needs some buttons)
 * ***********************************************************************************************/
I_PopUp {
    id: _root

    /* Enums and Signals
     * ****************************************************************************************/
    //! This enum is used to add buttons to dialog. Note that at most 4 buttons is allowed
    enum DialogButton {
        Yes=0b0001,
        No=0b0010,
        Cancel=0b0100,
        Custom=0b1000
    }

    signal accepted()
    signal rejected()
    signal buttonClicked(int button)

    /* Property declaration
     * ****************************************************************************************/
    //! Description for this prompt
    property string     description: ""

    //! A flag of buttons in this dialog
    property int        buttons: (I_Dialog.DialogButton.Yes | I_Dialog.DialogButton.No)

    /* Object properties
     * ****************************************************************************************/
    title: "Continue ?"
    leftPadding: 24
    rightPadding: 24
    topPadding: 16
    bottomPadding: 16
    titleHeadingLevel: 4
    contentItem: ColumnLayout {
        spacing: 8
        //! Title
        Label {
            Layout.fillWidth: true
            wrapMode: "WordWrap"
            textFormat: "MarkdownText"
            text: `${"#".repeat(titleHeadingLevel)} ${title}`
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
            color: enabled ? Style.foreground : Style.hintTextColor
        }

        //! Details content
        Label {
            Layout.fillWidth: description.length > 0
            visible: description.length > 0
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
            wrapMode: "WordWrap"
            text: description
            color: enabled ? Style.foreground : Style.hintTextColor
        }

        //! Buttons
        Flow {
            Layout.preferredWidth: Math.min(implicitWidth, parent.width)
            Layout.alignment: Qt.AlignHCenter
            flow: Flow.LeftToRight
            spacing: 8

            Repeater {
                model: {
                    var m = [];
                    if ((buttons & I_Dialog.DialogButton.No))       m.push(buttons & I_Dialog.DialogButton.No);
                    if ((buttons & I_Dialog.DialogButton.Cancel))   m.push(buttons & I_Dialog.DialogButton.Cancel);
                    if ((buttons & I_Dialog.DialogButton.Custom))   m.push(buttons & I_Dialog.DialogButton.Custom);
                    if ((buttons & I_Dialog.DialogButton.Yes))      m.push(buttons & I_Dialog.DialogButton.Yes);

                    return m;
                }

                delegate: ButtonInverted {
                    required property var modelData
                    required property int index

                    verticalPadding: 4
                    text: _internal.buttonsText[`${modelData}`] ?? ""
                    onClicked: {
                        if (modelData === I_Dialog.DialogButton.Yes) {
                            accepted();
                        } else if (modelData === I_Dialog.DialogButton.No
                                   || modelData === I_Dialog.DialogButton.Cancel) {
                            rejected();
                        }

                        buttonClicked(modelData);
                    }
                }
            }
        }
    }

    QtObject {
        id: _internal

        //! This is the default text for buttons
        property var buttonsText: {
            "1": "Yes",
            "2": "No",
            "4": "Cancel",
            "8": "Button",
        }
    }

    onAccepted: close();
    onRejected: close();

    function setButtonText(button: int, text: string)
    {
        if (_internal.buttonsText[`${button}`]) {
            _internal.buttonsText[`${button}`] = text;
            _internal.buttonsTextChanged();
        }
    }
}

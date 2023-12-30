import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemAccessoriesPage provides a ui for controlling system accesories
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    //! SystemAccessories
    property SystemAccessories systemAccessories: appModel.systemSetup.systemAccessories

    property bool              isHumidifier:      systemAccessories.accessoriesType === AppSpecCPP.Humidifier

    /* Object properties
     * ****************************************************************************************/
    title: "Accessories"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            //! Apply settings and pop this from StackView
            //! Apply settings here
            var accTypeUI = (humidifierT1PWRD.checked || humidifierT1Short.checked || humidifierT2PWRD.checked) ?
                            AppSpecCPP.Humidifier : AppSpecCPP.Dehumidifier;

            var wireTypeUI = AppSpecCPP.None;

            if (!noneChbox.checked) {
                if (humidifierT1PWRD.checked || deHumidifierT1PWRD.checked) {
                    wireTypeUI = AppSpecCPP.T1PWRD;

                } else if (humidifierT2PWRD.checked || deHumidifierT2PWRD.checked) {
                    wireTypeUI = AppSpecCPP.T2PWRD;

                } else if (humidifierT1Short.checked || deHumidifierT1Short.checked) {
                    wireTypeUI = AppSpecCPP.T1Short;

                }
            }

            deviceController.setSystemAccesseories(accTypeUI, wireTypeUI);

            //! Also move out of this Page
            backButtonCallback();
        }
    }

    GridLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9
        columns: 3
        columnSpacing: 4
        rowSpacing: 4

        Label {
            Layout.columnSpan: 3
            text: "Humidifier"
        }

        //! Humidifier CheckBox 1
        RadioButton {
            id: humidifierT1PWRD

            Layout.leftMargin: 40 * scaleFactor
            checked: isHumidifier && systemAccessories.accessoriesWireType === AppSpecCPP.T1PWRD
            text: "T1\npwrd"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Humidifier CheckBox 2
        RadioButton {
            id: humidifierT1Short

            enabled: !noneChbox.checked
            checked: isHumidifier && systemAccessories.accessoriesWireType === AppSpecCPP.T1Short
            text: "T1\nshort"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Humidifier CheckBox 3
        RadioButton {
            id: humidifierT2PWRD

            checked: isHumidifier && systemAccessories.accessoriesWireType === AppSpecCPP.T2PWRD
            text: "T2\npwrd"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        Label {
            Layout.columnSpan: 3
            Layout.topMargin: 40 * scaleFactor
            text: "Dehumidifier"
        }

        //! Dehumidifier CheckBox 1
        RadioButton {
            id: deHumidifierT1PWRD

            Layout.leftMargin: 40 * scaleFactor
            checked: !isHumidifier && systemAccessories.accessoriesWireType === AppSpecCPP.T1PWRD
            text: "T1\npwrd"

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Dehumidifier CheckBox 2
        RadioButton {
            id: deHumidifierT1Short

            text: "T1\nshort"
            checked: !isHumidifier && systemAccessories.accessoriesWireType === AppSpecCPP.T1Short

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! Dehumidifier CheckBox 3
        RadioButton {
            id: deHumidifierT2PWRD

            text: "T2\npwrd"
            checked: !isHumidifier && systemAccessories.accessoriesWireType === AppSpecCPP.T2PWRD

            Component.onCompleted: contentItem.horizontalAlignment = Qt.AlignHCenter
        }

        //! None
        Label {
            Layout.columnSpan: 3
            text: "None"
        }

        RadioButton {
            id: noneChbox

            Layout.leftMargin: 40 * scaleFactor
            checked: systemAccessories.accessoriesWireType === AppSpecCPP.None

        }
    }
}

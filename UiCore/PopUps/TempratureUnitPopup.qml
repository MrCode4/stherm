import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * TempratureUnitPopup
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //!
    property UiSession uiSession

    //! App model
    property I_Device appModel: uiSession?.appModel ?? null

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 24; rightPadding: 24
    title: "\u00b0F / \u00b0C"


    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay
        anchors.fill: parent
        spacing: 12

        Button {
            Layout.fillWidth: true
            autoExclusive: true
            checkable: true
            checked: appModel?.setting?.tempratureUnit === AppSpec.TempratureUnit.Fah
            text: "Fahrenheit (\u00b0F)"
        }

        Button {
            Layout.fillWidth: true
            autoExclusive: true
            checkable: true
            checked: !appModel || appModel?.setting?.tempratureUnit === AppSpec.TempratureUnit.Cel
            text: "Celsius (\u00b0C)"

            onCheckedChanged: {
                if (!appModel) return;

                if (checked) {
                    if (appModel.setting.tempratureUnit !== AppSpec.TempratureUnit.Cel) {
                        appModel.setting.tempratureUnit = AppSpec.TempratureUnit.Cel;
                    }
                } else {
                    if (appModel.setting.tempratureUnit !== AppSpec.TempratureUnit.Fah) {
                        appModel.setting.tempratureUnit = AppSpec.TempratureUnit.Fah;
                    }
                }

                // this is internal for now , no need to push or lock
                //                uiSession.deviceController.updateEditMode(AppSpec.EMSettings);
                uiSession.deviceController.pushSettings();

                delayedClose.start();
            }
        }

        Timer {
            id: delayedClose
            interval: 250
            onTriggered: close();
        }
    }
}

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
    title: "24H / 12H"

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
            checked: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
            text: "12 Hour"
        }

        Button {
            Layout.fillWidth: true
            autoExclusive: true
            checkable: true
            checked: !appModel || appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour24
            text: "24 Hour"

            onCheckedChanged: {
                if (!appModel) return;

                if (checked) {
                    if (appModel.setting.timeFormat !== AppSpec.TimeFormat.Hour24) {
                        appModel.setting.timeFormat = AppSpec.TimeFormat.Hour24;
                    }
                } else {
                    if (appModel.setting.timeFormat !== AppSpec.TimeFormat.Hour12) {
                        appModel.setting.timeFormat = AppSpec.TimeFormat.Hour12;
                    }
                }

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

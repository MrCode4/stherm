import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ResidenceTypePage provides ui for choosing residence type and device location
 * ***********************************************************************************************/
InitialSetupBasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    title: "Residence Type"

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.5
        spacing: 12

        Repeater {
            model: Object.keys(AppSpec.residenceTypesNames)
            delegate: Button {
                //! Convert model data to number
                property int modelDataNum: Number(modelData)

                Layout.fillWidth: true
                text: AppSpec.residenceTypesNames[modelDataNum]
                autoExclusive: true
                checked: appModel?.residenceType === modelDataNum

                onClicked: {
                    if (appModel.residenceType !== modelDataNum) {
                        appModel.residenceType = modelDataNum;
                        appModel.deviceLocation = ""
                    }

                    nextPage();
                }
            }
        }
    }

    /* Functions
     * ****************************************************************************************/

    function nextPage() {

        if (!root.StackView.view || appModel.residenceType === AppSpec.ResidenceTypes.Unknown) {
            console.log("ResidenceTypePage.qml: Can not continue to DeviceLocationPage: ", appModel.residenceType)
            return;
        }

        root.StackView.view.push("qrc:/Stherm/View/SystemSetup/DeviceLocationPage.qml", {
                                     "uiSession": Qt.binding(() => uiSession),
                                     "initialSetup":  root.initialSetup
                                 });
    }
}

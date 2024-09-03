import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ResidenceTypePage provides ui for choosing residence type and device location
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "Residence Type"

    /* Children
     * ****************************************************************************************/

    //! Info button in initial setup mode.
    InfoToolButton {
        parent: root.header.contentItem
        visible: initialSetup

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                             "uiSession": Qt.binding(() => uiSession),
                                             "initialSetup": root.initialSetup
                                         });
            }

        }
    }

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
            return;
        }

        if (appModel.residenceType === AppSpec.ResidenceTypes.Garage) {
            root.StackView.view.push("qrc:/Stherm/View/SystemSetup/ThermostatNamePage.qml", {
                                         "uiSession": Qt.binding(() => uiSession),
                                         "initialSetup":  root.initialSetup
                                     });

        } else {
            root.StackView.view.push("qrc:/Stherm/View/SystemSetup/DeviceLocationPage.qml", {
                                         "uiSession": Qt.binding(() => uiSession),
                                         "initialSetup":  root.initialSetup
                                     });
        }
    }
}

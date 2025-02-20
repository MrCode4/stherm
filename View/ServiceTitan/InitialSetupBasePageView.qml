import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InitialSetupBasePageView: BasePageView for initial setup pages.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Show wifi button
    property bool showWifiButton: false

    property bool initialSetup: false

    /* Children
     * ****************************************************************************************/
    //! Info button in initial setup mode.
    InfoToolButton {
        parent: root.header.contentItem
        visible: initialSetup

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                             "uiSession": Qt.binding(() => root.uiSession)
                                         });
            }
        }

        //! Wifi status
        WifiButton {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.topMargin: -10
            visible: showWifiButton && !NetworkInterface.hasInternet

            z: 1

            onClicked: {
                //! Open WifiPage
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/WifiPage.qml", {
                                                 "uiSession": root.uiSession,
                                                 "initialSetup": root.initialSetup,
                                                 "nextButtonEnabled": false
                                             });
                }
            }
        }
    }
}

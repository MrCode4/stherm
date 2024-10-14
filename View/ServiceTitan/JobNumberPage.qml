import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * JobNumberPage: TOOD: Needs to be completed.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    //! Busy due to get the job number information
    property bool isBusy: false

    /* Object properties
     * ****************************************************************************************/
    title: "Job Number"

    onVisibleChanged: {
        if (!visible)
            retryTimer.stop();
    }

    /* Children
     * ****************************************************************************************/
    //! Info button in initial setup mode.
    InfoToolButton {
        parent: root.header.contentItem
        visible: initialSetup

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                             "uiSession": Qt.binding(() => uiSession)
                                         });
            }
        }

        //! Wifi status
        WifiButton {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.topMargin: -10
            visible: !NetworkInterface.hasInternet

            z: 1

            onClicked: {
                //! Open WifiPage
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/WifiPage.qml", {
                                                 "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup,
                                                 "nextButtonEnabled": false
                                             });
                }
            }
        }
    }


    GridLayout {
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        rowSpacing: 10
        columnSpacing: 8
        columns: 2

        Label {
            Layout.columnSpan: 2
            text: "Job number"
            font.pointSize: root.font.pointSize * 0.9
        }

        TextField {
            id: jobNumberTf

            Layout.preferredWidth: parent.width * 0.8

            placeholderText: "Input the job number"
            text: appModel?.serviceTitan?.jobNumber ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^\d+$/
            }

            onTextChanged: {
                isBusy = false;
                retryTimer.stop();
            }

            inputMethodHints: Qt.ImhPreferNumbers

            BusyIndicator {

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                height: 45
                width: 45
                visible: isBusy
                running: visible

                TapHandler {
                    enabled: isBusy && errorPopup.errorMessage.length > 0

                    onTapped: {
                        errorPopup.open();
                    }
                }

            }
        }

        Text {
            id: skipCheckText

            Layout.alignment: Qt.AlignVCenter

            text: jobNumberTf.text.length > 0 ? qsTr("Check") : qsTr("Skip")
            color: isBusy ?  Qt.darker("#43E0F8", 1.5) : "#43E0F8"

            TapHandler {
                enabled: !isBusy

                onTapped: {
                    appModel.serviceTitan.jobNumber = jobNumberTf.text;

                    if (jobNumberTf.text.length > 0) {
                        isBusy = true;

                        appModel.serviceTitan.isSTManualMode = false;

                        if (!retryTimer.running)
                            retryTimer.triggered();

                    } else {
                        // Skip
                        appModel.serviceTitan.isSTManualMode = true;
                        nextPage();
                    }
                }
            }
        }

        Label {
            width: jobNumberTf.width

            Layout.columnSpan: 2
            Layout.preferredWidth: parent.width * 0.8

            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.7
            text: "Enter the ServiceTitan Job number and click \"Check\" to auto-fill the fields." +
                  "If you don't have one, click \"Skip.\""
        }

        Item {
            id: spacer

            Layout.columnSpan: 2
            Layout.fillWidth: true

            height: root.height / 7
        }

        Text {
            id: warrantyReplacementText

            Layout.columnSpan: 2
            text: qsTr("Warranty Replacement")
            font.underline: true
            font.pointSize: root.font.pointSize * 0.9
            color: "#43E0F8"

            TapHandler {
                onTapped: {
                    // Go to the warranty page
                    if (root.StackView.view) {
                        root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/WarrantyReplacementPage.qml", {
                                                     "uiSession": uiSession
                                                 });
                    }
                }
            }
        }

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 35

            font.pointSize: Application.font.pointSize * 0.7
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment:Text.AlignBottom
            text: "Contact Nuve Support: (657) 626-4887 for issues."
        }
    }

    //! Temp connection to go to the next page.
    Connections {
        target: deviceController.sync
        enabled: root.visible && isBusy

        function onJobInformationReady(success: bool, data: var, error: string) {

            if (success) {
                isBusy = false;
                errorPopup.errorMessage = "";
                nextPage();

            } else {
                errorPopup.errorMessage = "Job number operation failed, " + error;

                if (retryTimer.retryCount < 5){
                    // Retry
                    retryTimer.start();
                    retryTimer.retryCount++;

                } else {
                    isBusy = false;
                    retryTimer.retryCount = 0;
                    errorPopup.open();
                }
            }
        }

    }

    Timer {
        id: retryTimer

        property int retryCount: 0

        interval: 5000
        repeat: false
        running: false

        onTriggered: {
            deviceController.sync.getJobIdInformation(appModel.serviceTitan.jobNumber);
        }
    }

    InitialFlowErrorPopup {
        id: errorPopup

        deviceController: uiSession.deviceController
    }

    /* Functions
     * ****************************************************************************************/

    //! Go to CustomerDetailsPage
    function nextPage() {
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/CustomerDetailsPage.qml", {
                                         "uiSession": uiSession,
                                         "initialSetup": root.initialSetup
                                     });
        }
    }
}

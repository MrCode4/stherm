import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * JobNumberPage: TOOD: Needs to be completed.
 * ***********************************************************************************************/
InitialSetupBasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Busy due to get the job number information
    property bool isBusy: false

    /* Object properties
     * ****************************************************************************************/
    title: "Job Number"

    showWifiButton: !deviceController.initialSetupNoWIFI

    onVisibleChanged: {
        if (!visible) {
            retryTimer.stop();
            errorPopup.close();
        }
    }

    /* Children
     * ****************************************************************************************/

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

            // if this is enabled when busy, changing the value will stop retrying
            enabled: !isBusy
            placeholderText: "Input the job number"
            text: appModel?.serviceTitan?.jobNumber ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^\d+$/
            }

            //! can not be changed for now while isBusy,
            //! kept for future usage when is always enabled
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
                        if (NetworkInterface.hasInternet) {
                            isBusy = true;

                            appModel.serviceTitan.isSTManualMode = false;

                            retryTimer.stop();
                            retryTimer.triggered();

                        } else {
                            errorPopup.errorMessage = deviceController.deviceInternetError();
                            errorPopup.open();
                        }

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

        ContactNuveSupportLabel {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 35
        }
    }

    //! Temp connection to go to the next page.
    Connections {
        target: deviceController.sync
        enabled: root.visible && isBusy

        function onJobInformationReady(success: bool, data: var, error: string, isNeedRetry: bool) {
            isBusy = !success && isNeedRetry;

            if (success) {
                retryTimer.retryCount = 0;
                errorPopup.errorMessage = "";
                errorPopup.close();

                nextPage();

            } else {
                errorPopup.errorMessage = "Job number operation failed, " + error;

                if (isNeedRetry) {
                    // Retry
                    retryTimer.start();
                }

                if (!isNeedRetry || (retryTimer.retryCount % 2 === 0)) {
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
            retryCount++;
            deviceController.sync.getJobIdInformation(appModel.serviceTitan.jobNumber);
        }
    }

    CriticalErrorDiagnosticsPopup {
        id: errorPopup

        isBusy: root.isBusy
        deviceController: uiSession.deviceController

        onStopped: {
            root.isBusy = false;
            retryTimer.stop();
        }

        Text {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 10
            anchors.bottomMargin: 35

            visible: !isBusy
            text: qsTr("Skip")
            color: "#43E0F8"

            TapHandler {
                enabled: !isBusy

                onTapped: {
                    // Skip
                    appModel.serviceTitan.isSTManualMode = true;
                    nextPage();
                }
            }
        }
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

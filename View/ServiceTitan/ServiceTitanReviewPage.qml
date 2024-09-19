import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ServiceTitanReviewPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    property bool fetchMnaual: appModel?.serviceTitan?.isSTManualMode ?? true

    //! Busy due to multiple api calls
    property bool isBusyZip: false
    property bool isBusyCustomer: false
    readonly property bool isBusy: isBusyZip || isBusyCustomer

    /* Object properties
     * ****************************************************************************************/
    title: "Review"

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
    }

    Label {
        id: confirmInfoLabel

        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.9
        text: "Please confirm with the customer that the information below is correct."
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }

    ColumnLayout {
        anchors.top: confirmInfoLabel.bottom
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        spacing: 4

        Label {
            text: "Email"
            font.pointSize: root.font.pointSize
        }

        Label {

            id: emailTf

            text: appModel?.serviceTitan?.email ?? ""
            font.pointSize: Application.font.pointSize * 0.9
        }

        Item {
            Layout.fillWidth: true
            height: 20
        }

        Label {
            text: "ZIP code"
            font.pointSize: root.font.pointSize
        }

        Label {
            id: zipCodeTf

            Layout.fillWidth: true

            text: appModel?.serviceTitan?.zipCode ?? ""
            font.pointSize: Application.font.pointSize * 0.9
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    //! Next button
    ButtonInverted {
        id: nextBtn

        enabled: !isBusy

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        text: "Next"
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            isBusyCustomer = true;
            isBusyZip = true;
            // get needed values from api
            deviceController.getJobInformationManual();
        }
    }

    BusyIndicator {
        anchors.right: nextBtn.left
        anchors.verticalCenter: nextBtn.verticalCenter

        height: 45
        width: 45
        visible: isBusy
        running: visible
    }

    Label {
        id: errorLabel

        anchors.bottom: nextBtn.top
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter

        font.pointSize: root.font.pointSize * 0.7
        text: ""
        color: AppStyle.primaryRed
        visible: text.length > 0 && !isBusy
    }

    //! Temp connection to go to the next page.
    Connections {
        target: deviceController
        enabled: root.visible
        function onCustomerInfoReady(error) {
            if (error.length > 0) {
                errorLabel.text = error + " retry.";
            }

            isBusyCustomer = false;

            nextPage();
        }

        function onZipCodeInfoReady(error) {
            if (error.length > 0) {
                errorLabel.text = error + " retry.";
            }

            isBusyZip = false;

            nextPage();
        }
    }

    /* Functions
     * ****************************************************************************************/

    //! Go to CustomerDetailsPage
    function nextPage() {
        //! we prevent to go next page as one of calls is busy or has error
        if (errorLabel.text.length > 0 || isBusy)
            return;

        // Go to next page
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/SystemSetup/InstallationTypePage.qml", {
                                          "uiSession": uiSession,
                                         "initialSetup": root.initialSetup
                                      });
        }
    }

}

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ManageEndpoint
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    property string subdomain: "devapi";
    property string domain: "nuvehvac.com";


    /* Object properties
     * ****************************************************************************************/
    leftPadding: 8 * scaleFactor
    rightPadding: 12 * scaleFactor
    title: "Change Endpoint"
    backButtonCallback: function() {
        //! Check if domain is modified
        if (subdomain === subdomainTF.text && domain === domainTF.text) {
            tryGoBack()

        } else {
            //! This means that changes are occured that are not saved into model
            uiSession.popUps.exitConfirmPopup.accepted.connect(confirmtBtn.clicked);
            uiSession.popUps.exitConfirmPopup.rejected.connect(tryGoBack);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.exitConfirmPopup);
        }
    }

    Component.onCompleted: {
        var host = deviceController.deviceControllerCPP.getEndpoint();
        var hostParts = host.split('.')
        if (hostParts.length > 2) {
            subdomain = hostParts[0] // First part is subdomain
            domain = hostParts.slice(1).join('.') // Rest is the domain
        } else { //if (hostParts.length == 2) {
            // No subdomain, only domain and TLD
            subdomain = "";
            domain = host;
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        id: confirmtBtn
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! Save
            deviceController.deviceControllerCPP.setEndpoint(subdomainTF.text, domainTF.text);

            tryGoBack()
        }
    }

    Flickable {
        id: _contentFlick

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: _root.contentItem.y
            parent: _root
            height: _root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.bottom: resetButton.top
        anchors.rightMargin: 10
        clip: true
        contentWidth: width
        contentHeight: _contentLay.implicitHeight + 4
        boundsMovement: Flickable.StopAtBounds

        GridLayout {
            id: _contentLay
            width: parent.width
            columns: 1
            columnSpacing: 16 * scaleFactor
            rowSpacing: 8 * scaleFactor

            Label {
                text: "Endpoint:"
            }

            RowLayout {
                TextField {
                    id: subdomainTF
                    Layout.fillWidth: true
                    placeholderText: "devapi"
                    text: subdomain
                }
                Label {
                    text: "."
                }
                TextField {
                    id: domainTF
                    Layout.fillWidth: true
                    placeholderText: "nuvehvac.com"
                    text: domain
                }
            }
        }
    }

    //! Reset setting Button
    ButtonInverted {
        id: resetButton
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10
        text: "Reset"
        onClicked: {
            subdomainTF.text = "devapi";
            domainTF.text = "nuvehvac.com";
        }
    }
}

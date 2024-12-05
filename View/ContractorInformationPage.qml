import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ContractorInformationPage: Get and show the contractor information page.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property delcaration
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    //! Next/Confirm button
    ToolButton {
        parent: root.header.contentItem

        RoniaTextIcon {
            anchors.centerIn: parent
            text: FAIcons.arrowRight
        }

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/ContractorInformationFinishPage.qml", {
                                             "uiSession": uiSession
                                         });
            }
        }

    }

    Flickable {
        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: _contentCol.implicitHeight

        GridLayout {
            id: _contentCol

            columns: 2
            columnSpacing: 8
            rowSpacing: 8

            Label {
                text: "Serial No "
                visible: deviceController.system.serialNumber.length > 0
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: deviceController.system.serialNumber
                visible: deviceController.system.serialNumber.length > 0
                font.pixelSize: Qt.application.font.pointSize * 0.8
                horizontalAlignment: Qt.AlignLeft
            }
            Label {
                text: "Brand name "
                visible: appModel.contactContractor.brandName.length > 0
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: appModel.contactContractor.brandName
                visible: appModel.contactContractor.brandName.length > 0
                font.pixelSize: Qt.application.font.pointSize * 0.8
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: "Phone number: "
                visible: appModel.contactContractor.phoneNumber.length > 0
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: appModel.contactContractor.phoneNumber
                visible: appModel.contactContractor.phoneNumber.length > 0
                font.pixelSize: Qt.application.font.pointSize * 0.8
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: "QR URL: "
                visible: appModel.contactContractor.qrURL.length > 0
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: appModel.contactContractor.qrURL
                visible: appModel.contactContractor.qrURL.length > 0
                font.pixelSize: Qt.application.font.pointSize * 0.8
                horizontalAlignment: Qt.AlignLeft
            }

            //! Organization icon
            OrganizationIcon {
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter

                appModel: root.appModel
                width: root.width * 0.5
                height: root.height * 0.25
            }

            ButtonInverted {
                Layout.fillWidth: true
                text: "Update Contractor Info"

                onClicked: {
                    getContractorInfo.stop();
                    getContractorInfo.triggered();
                }
            }

        }
    }

    Timer {
        id: getContractorInfo
        repeat: false
        running: false
        interval: 10000

        onTriggered: {
            deviceController.deviceControllerCPP.checkContractorInfo();
        }
    }

    property Connections systemConnection: Connections {
        target: deviceController.system

        enabled: root.visible

        function onContractorInfoReady(getDataFromServerSuccessfully : bool) {
            if (getDataFromServerSuccessfully)
                fetchContractorInfoTimer.stop();
            else
                fetchContractorInfoTimer.restart();
        }
    }
}

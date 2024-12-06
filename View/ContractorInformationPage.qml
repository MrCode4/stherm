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

    property bool isBusy: false

    /* Object properties
     * ****************************************************************************************/

    title: "Contractor Information"

    onVisibleChanged: {
        isBusy = visible;

        if (visible) {
            getContractorInfoTimer.triggered();
        } else {
            getContractorInfoTimer.stop();
        }
    }

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

            width: parent.width
            columns: 2
            columnSpacing: 8
            rowSpacing: 8

            Label {
                text: "Serial No "
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: deviceController.system.serialNumber
                font.pixelSize: Qt.application.font.pointSize * 0.9
                horizontalAlignment: Qt.AlignLeft
            }
            Label {
                text: "Brand name "
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: appModel.contactContractor.brandName
                font.pixelSize: Qt.application.font.pointSize * 0.9
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: "Phone number "
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                text: appModel.contactContractor.phoneNumber
                font.pixelSize: Qt.application.font.pointSize * 0.9
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                Layout.columnSpan: 2

                text: "QR URL "
                font.pixelSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Qt.AlignLeft
            }

            Label {
                Layout.columnSpan: 2

                text: appModel.contactContractor.qrURL
                font.pixelSize: Qt.application.font.pointSize * 0.9
                wrapMode: Text.WordWrap
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

            Item {
                id: spacer

                Layout.columnSpan: 2
                height: 35
                width: 35
            }

            ButtonInverted {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.columnSpan: 2
                enabled: !isBusy

                text: "Update Contractor Info"

                onClicked: {
                    getContractorInfoTimer.stop();
                    getContractorInfoTimer.triggered();
                }
            }

        }
    }

    BusyIndicator {
        id: busyIndicator

       anchors.bottom: parent.bottom
       anchors.horizontalCenter: parent.horizontalCenter
       anchors.bottomMargin: 70

        height: 45
        width: 45
        visible: isBusy
        running: isBusy
    }

    Timer {
        id: getContractorInfoTimer
        repeat: false
        running: false
        interval: 10000

        onTriggered: {
            isBusy = true;
            deviceController.deviceControllerCPP.checkContractorInfo();
        }
    }

    property Connections systemConnection: Connections {
        target: deviceController.system

        enabled: root.visible

        function onContractorInfoReady(getDataFromServerSuccessfully : bool) {
            isBusy = !getDataFromServerSuccessfully;
            if (getDataFromServerSuccessfully)
                getContractorInfoTimer.stop();
            else
                getContractorInfoTimer.restart();
        }
    }
}

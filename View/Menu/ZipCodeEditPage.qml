import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ZipCodeEditPage provides ui to edit the country and related zip code.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property ServiceTitan serviceTitan: appModel.serviceTitan

    /* Object properties
     * ****************************************************************************************/
    title: "Location Hub"
    leftPadding: 10 * scaleFactor
    rightPadding: 10 * scaleFactor

    backButtonCallback: function() {
        //! Check if the user data changed
        if (zipCodeTf.text.toUpperCase() !== serviceTitan.zipCode.toUpperCase() ||
                serviceTitan?.country !== countryCombobox.currentText) {
            //! This means that changes are occured that are not saved into model
            uiSession.popUps.exitConfirmPopup.accepted.connect(confirmtBtn.clicked);
            uiSession.popUps.exitConfirmPopup.rejected.connect(goBack);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.exitConfirmPopup);

        } else {
            goBack();
        }
    }


    /* Children
     * ****************************************************************************************/

    ToolButton {
        id: confirmtBtn
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! TODO: Push the new data and show errors to user

            goBack();
        }
    }

    GridLayout {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        width: root.availableWidth

        columns: 2
        columnSpacing: 15
        rowSpacing: 0

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            text: "The zip code provided for your address is used to display accurate outdoor temperature information.\nPlease enter the correct information.\n\n"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Label {
            text: "Country"
            font.pointSize: root.font.pointSize * 0.9
        }

        Label {
            text: "ZIP code"
            font.pointSize: root.font.pointSize * 0.9
        }

        ComboBox {
            id: countryCombobox

            Layout.preferredWidth: root.availableWidth / 2 - 10
            Layout.alignment: Qt.AlignBottom

            font.pointSize: root.font.pointSize * 0.8
            model: AppSpec.supportedCountries
            currentIndex: appModel?.serviceTitan?.country.length > 0 ?
                              AppSpec.supportedCountries.indexOf(appModel?.serviceTitan?.country) : 0
        }

        TextField {
            id: zipCodeTf

            Layout.preferredWidth: root.availableWidth  / 2 - 10

            placeholderText: "Input the ZIP code"
            text: appModel?.serviceTitan?.zipCode.toUpperCase() ?? ""
            font.pointSize: root.font.pointSize * 0.8

            // Australia: 4 digits
            // Canada: 6 letters + digits
            // US: 5 digits: 10498
            validator: RegularExpressionValidator {
                regularExpression: /^(?:\d{4,5}|[A-Z\d]{6})$/i
            }

            inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhPreferUppercase
        }
    }

    /* Functions
     * ****************************************************************************************/

    //! This method is used to go back
    function goBack()
    {
        uiSession.popUps.exitConfirmPopup.accepted.disconnect(confirmtBtn.clicked);
        uiSession.popUps.exitConfirmPopup.rejected.disconnect(goBack);

        if (root.StackView.view) {
            //! Then Page is inside an StackView
            if (root.StackView.view.currentItem == root) {
                root.StackView.view.pop();
            }
        }
    }
}

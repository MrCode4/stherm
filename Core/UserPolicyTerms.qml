import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * UserPolicyTerms: Keep all private policy and terms of use
 * ************************************************************************************************/

QSObject {

    property string privacyPolicy: AppSpec.readFromFile(":/Stherm/Resources/privacyPolicy.md");

    property string termsOfUse:    AppSpec.readFromFile(":/Stherm/Resources/termOfUse.md");

    // Use one version for both (privacyPolicy and termsOfUse) for now
    property string currentVersion: "1"

    //! Accepted in normal mode
    property string acceptedVersion: ""

    //! Accepted in test mode
    property string acceptedVersionOnTestMode: ""
}

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QTemplateProject

/*! ***********************************************************************************************
 * About Window
 * ************************************************************************************************/
I_PopUp {
    id: popup

    /* Property Declarations
     * ****************************************************************************************/

    /* Object Properties
     * ****************************************************************************************/
    
    titleBar: true
    title: "ABOUT"
    width: 650
    height: 400

    /* Children
     * ****************************************************************************************/

    //! Logo
    Image {
        id: logoImage
//        source: "qrc:/QTemplateProject/Images/Logo.png"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.rightMargin: 30
    }

    //! Company
    Text {
        id: brandText
        text: qsTr("")
        font.pixelSize: 30
        color: "white"
        anchors.left: parent.left
        anchors.top: logoImage.bottom
        anchors.margins: 20
        anchors.leftMargin: 30
    }

    //! Version
    Text {
        id: versionText
        text: qsTr("Version 0.0.1")
        font.pixelSize: 20
        color: "white"
        anchors.left: parent.left
        anchors.top: brandText.bottom
        anchors.margins: 7
        anchors.leftMargin: 30
    }

    //! Copyright Text
    Text {
        id: copyrightText
        text: qsTr("")
        font.pixelSize: 20
        color: "white"
        anchors.left: parent.left
        anchors.top: versionText.bottom
        anchors.margins: 7
        anchors.leftMargin: 30
    }

    //! Description Text
    Text {
        id: descriptionText
        text: qsTr("")
        font.pixelSize: 14
        width: parent.width * 0.8
        anchors.horizontalCenter: parent.horizontalCenter
        wrapMode: Text.Wrap
        color: "white"
        anchors.top: copyrightText.bottom
        anchors.margins: 25
        anchors.leftMargin: 30
    }
}

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * Home page of the application
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 480
    implicitHeight: 480

    /* Children
     * ****************************************************************************************/
    //! Current temprature item
    CurrentTempratureLabel {
        id: _currentTempLbl
        anchors {
            left: parent.left
            top: parent.top
        }
        z: 1
    }

    //! Wifi status
    WifiButton {
        id: _wifiBtn
        anchors {
            right: parent.right
            top: parent.top
        }
        z: 1
    }

    //! Desired temprature slider and value
    DesiredTempratureItem {
        id: _desiredTempItem
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height / 2
        width: parent.availableWidth
    }

    //! Operation mode button
    OperationModeButton {
        anchors {
            centerIn: _desiredTempItem
            horizontalCenterOffset: -width - 48
        }
    }

    //! Other items
    GridLayout {
        anchors {
            verticalCenter: _desiredTempItem.bottom
            horizontalCenter: parent.horizontalCenter
            verticalCenterOffset: 24
        }
        columns: 3
        rowSpacing: 56

        //! Humidity item
        HumidityLabel {
            Layout.alignment: Qt.AlignCenter

        }

        //! Date and Timer
        DateTimeLabel {
            Layout.rowSpan: 2
            Layout.alignment: Qt.AlignCenter
        }

        //! Air condition item
        AirConditionItem {
            Layout.alignment: Qt.AlignCenter

        }

        //! Fan
        FanButton {
            Layout.alignment: Qt.AlignCenter

        }

        //! Hold button
        HoldButton {
            Layout.alignment: Qt.AlignCenter
        }
    }

    //! NEXGEN icon

}

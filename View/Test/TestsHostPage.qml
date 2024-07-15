import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * All Tests Host Page
 * ***********************************************************************************************/

BasePageView {
    header.visible: false
    property SimpleStackView testsStackView: stackView

    SimpleStackView {
        id: stackView
        anchors.fill: parent
    }

    Component.onCompleted: {
        testsStackView.push("qrc:/Stherm/View/Test/StartTestPage.qml", {"uiSession": Qt.binding(() => uiSession)});
    }
}

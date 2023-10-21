import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * The central widget of the application top level gui.
 *
 * ************************************************************************************************/
Item {
    id: mainView

    /* Property Declarations
     * ****************************************************************************************/
    property UiSession  uiSession

    /* Children
     * ****************************************************************************************/
    implicitWidth: 480
    implicitHeight: 480

    StackView {
        id: _mainStackView
        anchors.fill: parent

        Component.onCompleted: {
            //! Push _mainViewSw to stack here, to make sure Home is current item without SwipeView
            //! animating at window showing up.
            _mainStackView.push(_mainViewSw);
        }
    }

    //! Home, FanPage and HumidityPage SwipeView
    SwipeView {
        id: _mainViewSw
        currentIndex: 1

        FanPage {
            uiSession: mainView.uiSession
            backButtonVisible: false
        }

        Home {
            uiSession: mainView.uiSession
        }

        HumidityPage {
            uiSession: mainView.uiSession
            backButtonVisible: false
        }
    }
}

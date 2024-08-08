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

    //! unlockPage, use property to avoid delete the page in pop of stack view.
    property UnlockPage unlockPage: UnlockPage {
        uiSession: mainView.uiSession
    }

    /* Children
     * ****************************************************************************************/
    implicitWidth: 480
    implicitHeight: 480

    StackView {
        id: _mainStackView
        anchors.fill: parent
        initialItem: _homePage

        /*Component.onCompleted: {
            //! Push _mainViewSw to stack here, to make sure Home is current item without SwipeView
            //! animating at window showing up.
            _mainStackView.push(_mainViewSw);
        }*/
    }

    //! Home
    Home {
        id: _homePage
        mainStackView: _mainStackView
        uiSession: mainView.uiSession
    }

    /*SwipeView {
        id: _mainViewSw
        currentIndex: 1
        interactive: !_homePage.isDragging

        //! This should be sync either with the model (I_Device) or with the FanPage used in
        //! ApplicationMenu to prevent further issues or use exactly this FanPage instance in
        //! ApplicationMenu to push to _mainStackView.
        FanPage {
            uiSession: mainView.uiSession
            backButtonVisible: false
        }

        Home {
            id: _homePage
            mainStackView: _mainStackView
            uiSession: mainView.uiSession
        }

        HumidityPage {
            uiSession: mainView.uiSession
            backButtonVisible: false
        }
    }*/

    Connections {
        target: uiSession

        function onShowHome()
        {
            if (!ScreenSaverManager.isAppActivated()) {
                console.log("The app must be active before you can access the home page.")
                return;
            }

            //! Avoid to close the SystemUpdatePage on mandatory update mode
            if (!uiSession.deviceController.mandatoryUpdate || !(_mainStackView.currentItem instanceof SystemUpdatePage))
                _mainStackView.pop(null) //! Pop all items except the first one which is _mainViewSw

            //! Close all popups too.
            uiSession.popupLayout.closeAllPopups();

            // Touching the screen should prompt a page requesting a 4-digit PIN for unlocking (Unlock page)
            if (uiSession.appModel.lock.isLock) {
                _mainStackView.push(unlockPage);
            }
        }
    }
}

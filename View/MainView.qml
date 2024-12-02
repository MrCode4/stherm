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
    readonly property int stackViewDepth: _mainStackView.depth

    //! unlockPage, use property to avoid delete the page in pop of stack view.
    property UnlockPage unlockPage: UnlockPage {
        uiSession: mainView.uiSession
    }

    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size

    StackView {
        id: _mainStackView
        anchors.fill: parent
        initialItem: _homePage
    }

    Home {
        id: _homePage
        mainStackView: _mainStackView
        uiSession: mainView.uiSession
    }

    Connections {
        target: uiSession

        function onOpenUnlockPage() {
            // TODO
            // _mainStackView.push(unlockPage);
        }

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

import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

BasePageView {
    id: root

    PerfTestPopup {
        id: perfTestPopup
        uiSession: root.uiSession
    }

    Component.onCompleted: {
        uiSession.popupLayout.closeAllPopups();
        uiSession.popupLayout.displayPopUp(perfTestPopup);
    }
    Component.onDestruction: {
        console.log("Destructing PerfTestPage")
        perfTestPopup.close()
    }
}

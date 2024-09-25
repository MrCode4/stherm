import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

I_PopUp {
    id: root
    leftPadding: 24; rightPadding: 24
    title: "Performance Test"

    property UiSession uiSession
    property I_Device appModel: uiSession?.appModel ?? null

}

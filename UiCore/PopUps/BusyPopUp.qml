import QtQuick
import QtQuick.Controls.Material

import Stherm

I_PopUp {
    id: root

    closePolicy: I_PopUp.NoAutoClose
    topPadding: 24
    bottomPadding: 24
    horizontalPadding: 24

    contentItem: BusyIndicator {}
}

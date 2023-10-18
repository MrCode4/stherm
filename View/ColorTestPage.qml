import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ColorTestPage is a page to show selected color of BacklightPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    Material.theme: Material.background.hslLightness < 0.55 ? Material.Dark : Material.Light
    title: "Color Test"
}

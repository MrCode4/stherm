import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ApplicationMenu provides ui to access different section of application
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Menu"
    contentItem: ApplicationMenuList {
        onMenuActivated: function(menuTitle) {
            //! Push related menu to stack
            if (_root.StackView.view) {
                switch(menuTitle) {
                case "Backlight":
                    _root.StackView.view.push("qrc:/Stherm/View/BacklightPage.qml",
                                              {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
                    break;
                case "Fan Control":
                    _root.StackView.view.push("qrc:/Stherm/View/FanPage.qml",
                                              {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
                    break;
                case "Schedule":
                    _root.StackView.view.push("qrc:/Stherm/View/ScheduleView.qml", {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
                    break;
                case "Humidity Control":
                    _root.StackView.view.push("qrc:/Stherm/View/HumidityPage.qml", {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
                    break;
                case "Settings":
                    _root.StackView.view.push("qrc:/Stherm/View/SettingsPage.qml", {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
                    break;
                case "System Setup":
                    _root.StackView.view.push("qrc:/Stherm/View/SystemSetupPage.qml", {
                                                  "uiSession": Qt.binding(() => uiSession)
                                              });
                    break;
                case "System Info":
                    _root.StackView.view.push("qrc:/Stherm/View/SystemInfoPage.qml")
                    break;
                }
            }
        }
    }
}
